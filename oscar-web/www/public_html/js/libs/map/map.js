define(["require", "state", "jquery", "conf", "oscar", "flickr", "tools", "tree", "bootstrap", "spinner"], function (require, state, $, config, oscar, flickr, tools, tree) {
    spinner = require("spinner");

    return map = {
        addShapeToMap: function (leafletItem, itemId, shapeSrcType) {
            var itemDetailsId = '#' + shapeSrcType + 'Details' + itemId;
            var itemPanelRootId = '#' + shapeSrcType + 'PanelRoot' + itemId;
            if (config.functionality.shapes.highlightListItemOnClick[shapeSrcType]) {
                leafletItem.on('click', function () {

                    if ($('#show_flickr').is(':checked')) {
                        var geopos;

                        if (this instanceof L.MultiPolygon) {
                            geopos = this.getLatLngs()[0][0];
                        } else if (this instanceof L.Polygon) {
                            geopos = this.getLatLngs()[0];
                        } else if (this instanceof L.Polyline) {
                            geopos = this.getLatLngs()[0];
                        } else {
                            geopos = this.getLatLng();
                        }
                        flickr.getImagesForLocation($.trim(state.DAG.at(itemId).name), geopos);
                    }

                    state.sidebar.open("search");
                    // open a tab, that contains the element
                    var parentId = state.DAG.at(itemId).parents[0].id;
                    var index = $("#tabs a[href='#tab-" + parentId + "']").parent().index();

                    $("#items_parent").tabs("option", "active", index);

                    $('#' + shapeSrcType + "List").find('.panel-collapse').each(
                        function () {
                            if ($(this).hasClass('in')) {
                                $(this).collapse('hide');
                            }
                        }
                    );
                    $(itemDetailsId).collapse('show');
                    //var container = $('#'+ shapeSrcType +'_parent');
                    var container = $(".sidebar-content");
                    var itemPanelRootDiv = $(itemPanelRootId);
                    if (itemPanelRootDiv === undefined) {
                        console.log("addShapeToMap: undefined PanelRoot", leafletItem, itemId, shapeSrcType, state);
                    }
                    else {
                        var scrollPos = itemPanelRootDiv.offset().top - container.offset().top + container.scrollTop();
                        container.animate({scrollTop: scrollPos});
                        //container.animate({scrollTop: itemPanelRootDiv.position().top + $("itemsList").position().top});
                    }
                });
            }
        },

        visualizeResultListItems: function () {
            state.items.shapes.promised = {};
            var itemsToDraw = [];
            for (var i in state.items.listview.drawn.values()) {
                if (!state.items.shapes.drawn.count(i)) {
                    state.items.shapes.promised[i] = state.items.listview.drawn.at(i);
                    itemsToDraw.push(i);
                }
            }

            spinner.startLoadingSpinner();
            oscar.getShapes(itemsToDraw, function (shapes) {
                spinner.endLoadingSpinner();

                var marker;
                for (var i in itemsToDraw) {
                    var itemId = itemsToDraw[i];
                    if (state.items.shapes.promised[itemId] === undefined) {
                        continue;
                    }
                    if (shapes[itemId] === undefined || !state.items.listview.drawn.count(itemId) ||
                        state.items.shapes.drawn.count(itemId)) {
                        delete state.items.shapes.promised[itemId];
                        continue;
                    }

                    delete state.items.shapes.promised[itemId];
                    var itemShape = oscar.leafletItemFromShape(shapes[itemId]);
                    itemShape.setStyle(config.styles.shapes.items.normal);

                    if (itemShape instanceof L.MultiPolygon) {
                        marker = L.marker(itemShape.getLatLngs()[0][0]);
                    } else if (itemShape instanceof L.Polygon) {
                        marker = L.marker(itemShape.getLatLngs()[0]);
                    } else if (itemShape instanceof L.Polyline) {
                        marker = L.marker(itemShape.getLatLngs()[0]);
                    } else {
                        marker = L.marker(itemShape.getLatLng());
                    }

                    state.markers.addLayer(marker);
                    state.items.shapes.drawn.insert(itemId, itemShape);
                    state.DAG.at(itemId).marker = marker;
                    state.DAG.at(itemId).shape = itemShape;
                    map.addShapeToMap(marker, itemId, "items");
                }
            }, oscar.defErrorCB);
        },

        appendItemToListView: function (item, shapeSrcType, parentElement) {
            if (item === undefined) {
                console.log("Undefined item displayed");
                return;
            }
            var itemId = item.id();
            state[shapeSrcType].listview.drawn.insert(itemId, item);

            var itemTemplateData = state.resultListTemplateDataFromItem(item, shapeSrcType);
            var rendered = $.Mustache.render('itemListEntryHtmlTemplate', itemTemplateData);
            var inserted = $($(rendered).appendTo(parentElement));
            $('#' + shapeSrcType + 'NameLink' + itemId, inserted).click(
                function () {
                    state.map.fitBounds(state[shapeSrcType].shapes.drawn.at(itemId).getBounds());
                    $('#' + shapeSrcType + "List").find('.panel-collapse').each(
                        function () {
                            if ($(this).hasClass('in')) {
                                $(this).collapse('hide');
                            }
                        }
                    );
                    var geopos;
                    var itemShape = state.items.shapes.drawn.at(itemId);
                    if (itemShape instanceof L.MultiPolygon) {
                        geopos = itemShape.getLatLngs()[0][0];
                    } else if (itemShape instanceof L.Polygon) {
                        geopos = itemShape.getLatLngs()[0];
                    } else if (itemShape instanceof L.Polyline) {
                        geopos = itemShape.getLatLngs()[0];
                    } else {
                        geopos = itemShape.getLatLng();
                    }

                    L.popup({offset: new L.Point(0, -25)})
                        .setLatLng(geopos)
                        .setContent($(this).text()).openOn(state.map);

                    if ($('#show_flickr').is(':checked')) {
                        flickr.getImagesForLocation($.trim($(this).text()), geopos);
                    }

                }
            );

            function itemDetailQuery(e) {
                var me = $(this);
                var myKey = me.attr('data-query-key');
                if (myKey === undefined) {
                    return false;
                }
                var myQstr = "@" + myKey;
                var myValue = me.attr('data-query-value');
                if (myValue !== undefined) {
                    myQstr += ":" + myValue;
                }
                tools.addSingleQueryStatementToQuery(myQstr);
                return false;
            }

            $('#' + shapeSrcType + 'Details' + itemId + " .item-detail-key", inserted).click(itemDetailQuery);
            $('#' + shapeSrcType + 'Details' + itemId + " .item-detail-value", inserted).click(itemDetailQuery);
        },

        flatCqrTreeDataSource: function (cqr) {
            function getItems(regionChildrenInfo, context) {
                var regionChildrenApxItemsMap = {};
                var childIds = [];
                var parentRid = context.rid;
                var parentNode = state.DAG.at(parentRid);
                var parentCount = parentNode.count;

                for (var i in regionChildrenInfo) {
                    var ci = regionChildrenInfo[i];
                    regionChildrenApxItemsMap[ci['id']] = ci['apxitems'];
                    childIds.push(ci['id']);
                }

                oscar.getItems(childIds,
                    function (items) {
                        var itemMap = {}, node, item, itemId, marker;

                        // modify DAG
                        if (!context.dynamic) {
                            for (var i in items) {
                                item = items[i];
                                itemId = item.id();
                                itemMap[itemId] = item;
                                if (!cqr.ohPath().length || ($.inArray(itemId, cqr.ohPath()) != -1 || (parentRid == cqr.ohPath()[cqr.ohPath().length - 1] && parentCount > oscar.maxFetchItems))) {
                                    if (!state.DAG.count(itemId)) {
                                        node = parentNode.addChild(itemId);
                                        marker = L.marker(item.centerPoint());
                                        node.count = marker.count = regionChildrenApxItemsMap[itemId];
                                        node.bbox = marker.bbox = item.bbox();
                                        node.name = marker.name = item.name();
                                        marker.rid = itemId;
                                        map.decorateMarker(marker);
                                        node.marker = marker;
                                        state.DAG.insert(itemId, node);
                                    } else {
                                        state.DAG.at(itemId).parents.push(parentNode);
                                        parentNode.children.push(state.DAG.at(itemId));
                                    }
                                }
                            }
                        } else if (parentCount > oscar.maxFetchItems) {
                            for (var i in items) {
                                item = items[i];
                                itemId = item.id();
                                itemMap[itemId] = item;
                                if (!state.DAG.count(itemId)) {
                                    node = parentNode.addChild(itemId);
                                    marker = L.marker(item.centerPoint());
                                    node.count = marker.count = regionChildrenApxItemsMap[itemId];
                                    node.bbox = marker.bbox = item.bbox();
                                    node.name = marker.name = item.name();
                                    marker.rid = itemId;
                                    map.decorateMarker(marker);
                                    node.marker = marker;
                                    state.DAG.insert(itemId, node);
                                } else {
                                    state.DAG.at(itemId).parents.push(parentNode);
                                    parentNode.children.push(state.DAG.at(itemId));
                                }
                            }
                        }

                        // DAG-modification finished, now dicide whether items should be loaded, or clusters be drawn
                        if (!items.length || (parentCount <= oscar.maxFetchItems)) {
                            if (context.draw || !cqr.ohPath().length || cqr.ohPath()[cqr.ohPath().length - 1] == parentRid) {
                                $("#left_menu_parent").css("display", "block");

                                if (cqr.ohPath().length) {
                                    state.items.listview.selectedRegionId = parentRid;
                                    state.cqr.regionItemIds(state.items.listview.selectedRegionId,
                                        map.getItemIds,
                                        oscar.defErrorCB,
                                        0 // offset
                                    );
                                } else {
                                    if (state.DAG.at(parentRid).children.length == 0) {
                                        state.items.listview.selectedRegionId = parentRid;
                                        state.cqr.regionItemIds(state.items.listview.selectedRegionId,
                                            map.getItemIds,
                                            oscar.defErrorCB,
                                            0 // offset
                                        );
                                    } else {
                                        for (var child in state.DAG.at(parentRid).children) {
                                            state.items.listview.selectedRegionId = state.DAG.at(parentRid).children[child].id;
                                            state.cqr.regionItemIds(state.items.listview.selectedRegionId,
                                                map.getItemIds,
                                                oscar.defErrorCB,
                                                0 // offset
                                            );
                                        }
                                    }
                                }
                            }
                        } else if (context.draw) {
                            state.items.clusters.drawn.erase(parentRid);

                            var j;
                            var regions = [];
                            for (var i in items) {
                                j = itemMap[items[i].id()];

                                if (!state.items.clusters.drawn.count(j.id())) {
                                    regions.push(j.id());
                                    marker = state.DAG.at(j.id()).marker;
                                    state.items.clusters.drawn.insert(j.id(), marker);
                                    state.markers.addLayer(marker);
                                    state.DAG.at(marker.rid).marker = marker;
                                }
                            }

                            // just load regionShapes into the cache
                            oscar.getShapes(regions, function (res) {
                            }, oscar.defErrorCB);

                            if (state.visualizationActive) {
                                tree.refresh(parentRid);
                            }
                        }

                        if (context.pathProcessor) {
                            context.pathProcessor.process();
                        }

                    },
                    oscar.defErrorCB
                );
            }

            return function (context) {
                spinner.startLoadingSpinner();
                cqr.regionChildrenInfo(context.rid, function (regionChildrenInfo) {
                        spinner.endLoadingSpinner()
                        getItems(regionChildrenInfo, context);
                    },
                    oscar.defErrorCB
                );
            };
        },

        getItemIds: function (regionId, itemIds) {

            for (var i in itemIds) {
                var itemId = itemIds[i];
                state.items.listview.promised.insert(itemId, itemId);
            }

            oscar.getItems(itemIds,
                function (items) {
                    var node;
                    state.items.clusters.drawn.erase(regionId);

                    // manage items -> kill old items if there are too many of them and show clusters again
                    if (state.items.listview.drawn.size() + items.length > config.maxBufferedItems) {
                        for (var i in state.items.listview.drawn.values()) {
                            node = state.DAG.at(i);
                            for (var parent in node.parents) {
                                if (!state.items.clusters.drawn.count(node.parents[parent].id)) {
                                    state.markers.addLayer(node.parents[parent].marker);
                                    state.items.clusters.drawn.insert(node.parents[parent].id, node.parents[parent].marker);
                                }
                            }
                            if (node.marker) {
                                state.markers.removeLayer(node.marker);
                            }
                            node.kill();
                            delete node;
                        }
                        $('#itemsList').empty();
                        $('#tabs').empty();
                        state.items.listview.drawn.clear();
                        state.items.shapes.drawn.clear();
                    }

                    var tabsInitialized = $('#items_parent').data("ui-tabs");
                    var tab = "<li><a href='#tab-" + regionId + "'>" + state.DAG.at(regionId).name + "</a><span class='badge'>" + items.length + "</span></li>";
                    if (!$("a[href='#tab-" + regionId + "']").length) {
                        $('#tabs').append(tab);
                    }

                    if (!tabsInitialized) {
                        $('#items_parent').tabs();
                    }

                    var regionDiv = "<div id='tab-" + regionId + "'></div>";
                    if (!$("#tab-" + regionId).length) {
                        $('#itemsList').append(regionDiv);
                    }
                    var parentElement = $('#tab-' + regionId);
                    for (var i in items) {
                        var item = items[i];
                        var itemId = item.id();
                        if (state.items.listview.promised.count(itemId)) {
                            if (!state.DAG.count(itemId)) {
                                state.DAG.insert(itemId, state.DAG.at(regionId).addChild(itemId));
                            } else {
                                //TODO: check whether regionId already contains this itemId as child
                                state.DAG.at(regionId).children.push(state.DAG.at(itemId));
                                state.DAG.at(itemId).parents.push(state.DAG.at(regionId));
                            }
                            state.DAG.at(itemId).name = item.name();
                            map.appendItemToListView(item, "items", parentElement);
                            state.items.listview.promised.erase(itemId);
                        }
                    }
                    map.refreshTabs();
                    map.visualizeResultListItems();

                    if (state.visualizationActive) {
                        tree.refresh(regionId);
                    }

                },
                oscar.defErrorCB
            );
        },

        loadSubhierarchy: function (rid, finish) {
            state.cqr.regionChildrenInfo(rid, function (regionChildrenInfo) {
                var children = [];
                var regionChildrenApxItemsMap = {};

                for (var i in regionChildrenInfo) {
                    var ci = regionChildrenInfo[i];
                    regionChildrenApxItemsMap[ci['id']] = ci['apxitems'];
                    children.push(ci['id']);
                }

                oscar.getItems(children, function (items) {
                        var itemId, item, node, parentNode, marker;
                        parentNode = state.DAG.at(rid);

                        for (var i in items) {
                            item = items[i];
                            itemId = item.id();
                            if (!state.DAG.count(itemId)) {
                                node = parentNode.addChild(itemId);
                                marker = L.marker(item.centerPoint());
                                node.count = marker.count = regionChildrenApxItemsMap[itemId];
                                node.bbox = marker.bbox = item.bbox();
                                node.name = marker.name = item.name();
                                marker.rid = item.id();
                                map.decorateMarker(marker);
                                node.marker = marker;
                                state.DAG.insert(itemId, node);
                                map.addClusterMarker(state.DAG.at(itemId));
                            }
                        }

                        if ($("#onePath").is(':checked')) {
                            tree.onePath(parentNode);
                        } else {
                            tree.refresh(rid);
                        }

                        finish();
                    }, function () {
                    }
                );
            }, function () {
            });
        },

        loadWholeTree: function () {
            function fullSubSetTreeDataSource(cqr) {
                state.DAG = tools.SimpleHash();
                var subSet = cqr.subSet();
                var regions = [];
                for (var region in subSet.regions) {
                    regions.push(region);
                }

                //fetch the items
                oscar.getItems(regions,
                    function (items) {
                        var marker;
                        for (var i in items) {
                            var item = items[i];
                            var itemId = item.id();
                            var regionInSubSet = subSet.regions[itemId];
                            var node = state.DAG.at(itemId);

                            if (node) {
                                marker = L.marker(item.centerPoint());
                                marker.rid = itemId;
                                node.name = marker.name = item.name();
                                node.count = marker.count = regionInSubSet.apxitems;
                                node.bbox = marker.bbox = item.bbox();
                                map.decorateMarker(marker);
                            } else {
                                var newNode = new tools.TreeNode(itemId, undefined);
                                newNode.count = regionInSubSet.apxitems;
                                newNode.name = item.name();
                                newNode.bbox = item.bbox();
                                state.DAG.insert(itemId, newNode);
                                node = newNode;
                            }

                            for (var child in regionInSubSet.children) {
                                node.addChild(regionInSubSet.children[child]);
                            }
                        }

                        var root = state.DAG.at(0xFFFFFFFF);
                        for (var j in subSet.rootchildren) {
                            root.children.push(state.DAG.at(subSet.rootchildren[j]));
                        }
                    },
                    oscar.defErrorCB
                );
            }

            var callFunc = function (q, scb, ecb) {
                oscar.completeFull(q, scb, ecb);
            };

            callFunc($("#search_text").val(),
                function (cqr) {
                    //orbit reached, iniate coupling with user
                    spinner.endLoadingSpinner();
                    state.cqr = cqr;
                    state.cqrRegExp = oscar.cqrRexExpFromQuery(cqr.query());
                    fullSubSetTreeDataSource(cqr);
                },
                function (jqXHR, textStatus, errorThrown) {
                    //BOOM!
                    spinner.endLoadingSpinner();
                    alert("Failed to retrieve completion results. textstatus=" + textStatus + "; errorThrown=" + errorThrown);
                });
        },

        loadItems: function (rid) {
            state.items.listview.selectedRegionId = rid;
            state.cqr.regionItemIds(state.items.listview.selectedRegionId,
                map.getItemIds,
                oscar.defErrorCB,
                0 // offset
            );
        },

        pathProcessor: function (cqr) {
            return {
                path: cqr.ohPath(),
                i: 0,
                process: function () {
                    var draw;
                    if (this.i < this.path.length) {
                        if (this.i != this.path.length - 1) {
                            draw = false;
                        } else {
                            draw = true;
                        }
                        this.i++;
                        state.regionHandler({rid: this.path[this.i - 1], draw: draw, pathProcessor: this});
                    } else {
                        // fit the viewport to the target region
                        if (this.path.length) {
                            state.map.fitBounds(state.DAG.at(this.path[this.path.length - 1]).bbox);
                        } else {
                            state.map.fitWorld();
                        }
                        state.handler = function () {
                            var timer = tools.timer("draw");
                            var drawn = tools.SimpleHash();
                            var removedParents = tools.SimpleHash();
                            var currentItemMarkers = state.items.shapes.drawn.values();
                            var currentClusterMarker = state.items.clusters.drawn.values();
                            var bulkMarkerBuffer = [];

                            for (var item in currentItemMarkers) {
                                drawn.insert(item, false);
                            }

                            for (var cluster in currentClusterMarker) {
                                map.removeClusterMarker(state.DAG.at(cluster));
                            }

                            if (this.path && this.path.length) {
                                // start at the target region (last element of ohPath)
                                map.drawClusters(state.DAG.at(this.path[this.path.length - 1]), drawn, bulkMarkerBuffer);
                            } else {
                                // start at Node "World"
                                map.drawClusters(state.DAG.at(0xFFFFFFFF), drawn, bulkMarkerBuffer);
                            }

                            state.markers.addLayers(bulkMarkerBuffer);

                            // remove all markers (and tabs) that are redundant
                            for (var item in drawn.values()) {
                                if (drawn.at(item) == false) {
                                    map.removeItemMarker(state.DAG.at(item));
                                    map.removeParentsTabs(state.DAG.at(item), removedParents);
                                }
                            }

                            timer.stop();
                        }.bind(this);
                        state.map.on("zoomend dragend", state.handler);
                    }
                }
            };
        },

        drawClusters: function (node, drawn, markerBuffer) {
            if (!node) {
                return;
            }

            var childNode;
            if (node.children.length) {
                for (var child in node.children) {
                    childNode = node.children[child];
                    if (tools.percentOfOverlap(state.map, childNode.bbox) >= config.overlap) {
                        map.drawClusters(childNode, drawn, markerBuffer);
                    } else {
                        if (childNode.count) {
                            map.addClusterMarker(childNode);
                        } else if (!childNode.count) {
                            if (!drawn.count(childNode.id)) {
                                map.addItemMarker(childNode);
                                map.setupTabsForItemAndAddToListView(childNode);
                            } else {
                                drawn.insert(childNode.id, true);
                            }
                        }
                    }
                }

            } else if (node.count) {
                state.regionHandler({rid: node.id, draw: true, dynamic: true});
            }

            map.refreshTabs();
        },

        displayCqr: function (cqr) {
            state.regionHandler = map.flatCqrTreeDataSource(cqr);
            var process = map.pathProcessor(cqr);
            var root = new tools.TreeNode(0xFFFFFFFF, undefined);
            root.count = cqr.rootRegionApxItemCount();
            root.name = "World";
            state.DAG.insert(0xFFFFFFFF, root);

            if (cqr.ohPath().length) {
                state.regionHandler({rid: 0xFFFFFFFF, draw: false, pathProcessor: process});
            } else {
                state.regionHandler({rid: 0xFFFFFFFF, draw: true, pathProcessor: process});
            }
        },

        refreshTabs: function () {
            var tabs = $('#items_parent');
            if (tabs.data("ui-tabs")) {
                tabs.tabs("refresh");
                tabs.tabs("option", "active", 0);
            }
        },

        initTabs: function () {
            var tabs = $('#items_parent');
            if (!tabs.data("ui-tabs")) {
                tabs.tabs();
            }
        },

        destroyTabs: function () {
            var tabs = $('#items_parent');
            if (tabs.data("ui-tabs")) {
                tabs.tabs("destroy");
            }
        },

        closePopups: function () {
            var closeElement = $(".leaflet-popup-close-button")[0];
            if (closeElement !== undefined) {
                closeElement.click();
            }
        },

        removeBoundaries: function () {
            for (var boundary in state.shownBoundaries) {
                state.map.removeLayer(state.shownBoundaries[boundary]);
            }
            state.shownBoundaries = [];
        },

        decorateMarker: function (marker) {
            marker.on("click", function (e) {
                map.closePopups();
                state.items.clusters.drawn.erase(e.target.rid);
                map.removeMarker(e.target);
                state.regionHandler({rid: e.target.rid, draw: true, dynamic: true});
            });

            marker.on("mouseover", function (e) {
                if (oscar.isShapeInCache(e.target.rid)) {
                    oscar.getShape(e.target.rid, function (shape) {
                        var leafletItem = oscar.leafletItemFromShape(shape);
                        leafletItem.setStyle(config.styles.shapes['regions']['normal']);
                        e.target.shape = leafletItem;
                        state.map.addLayer(leafletItem);
                    }, oscar.defErrorCB);
                }

                L.popup({offset: new L.Point(0, -10)})
                    .setLatLng(e.latlng)
                    .setContent(e.target.name).openOn(state.map);
            });

            marker.on("mouseout", function (e) {
                map.closePopups();
                if (e.target.shape) {
                    state.map.removeLayer(e.target.shape);
                }
            });
        },

        removeMarker: function (marker) {
            if (marker.shape) {
                state.map.removeLayer(marker.shape);
            }
            state.markers.removeLayer(marker);
            map.closePopups();
        },

        removeClusterMarker: function (node) {
            map.removeMarker(node.marker);
            state.items.clusters.drawn.erase(node.id);
            map.removeBoundaries();
        },

        addClusterMarker: function (node, buffer) {
            state.items.clusters.drawn.insert(node.id, node.marker);
            if (buffer) {
                buffer.push(node.marker)
            } else {
                state.markers.addLayer(node.marker);
            }
        },

        removeItemMarker: function (node) {
            state.markers.removeLayer(node.marker);
            state.items.shapes.drawn.erase(node.id);
            state.items.listview.drawn.erase(node.id);
        },

        addItemMarker: function (node, buffer) {
            state.items.shapes.drawn.insert(node.id, node.shape);
            if (buffer) {
                buffer.push(node.marker);
            } else if (node.marker) {
                state.markers.addLayer(node.marker);
            }
        },

        setupTabsForItemAndAddToListView: function (node) {
            map.initTabs();
            for (var parent in node.parents) {
                if (!$("a[href='#tab-" + node.parents[parent].id + "']").length) {
                    var tab = "<li><a href='#tab-" + node.parents[parent].id + "'>" + state.DAG.at(node.parents[parent].id).name + "</a><span class='badge'>" + node.parents[parent].count + "</span></li>";
                    $('#tabs').append(tab);
                }
                if (!$("#tab-" + node.parents[parent].id).length) {
                    var regionDiv = "<div id='tab-" + node.parents[parent].id + "'></div>";
                    $('#itemsList').append(regionDiv);
                }
                oscar.getItem(node.id, function (item) {
                    map.appendItemToListView(item, "items", $("#tab-" + node.parents[parent].id));
                }, oscar.defErrorCB);
            }
        },

        removeParentTab: function (childNode, parentNode, removedParents) {
            var parentTab = $("a[href='#tab-" + parentNode.id + "']");
            if (parentTab.length) {
                removedParents.insert(parentNode.id, parentNode.id);
                parentTab.parent().remove();
                $("#tab-" + parentNode.id).remove();
            }

        },

        removeParentsTabs: function (childNode, removedParents) {
            for (var parent in childNode.parents) {
                if (!removedParents.count(childNode.parents[parent].id)) {
                    map.removeParentTab(childNode, childNode.parents[parent], removedParents);
                }
            }

            if ($("#tabs").children().length > 0) {
                map.refreshTabs();
            } else {
                map.destroyTabs();
            }
        }
    };
})
;