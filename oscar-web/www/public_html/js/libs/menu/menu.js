define(["jquery"], function ($) {
    return {
        displayCategories: function () {
            var c = $("#categories");
            if (c.is(":empty")) {
                var container = $("<div class='list-group'></div>");
                for (var category in this.categories) {
                    $("<a style='border: none' class='list-group-item'><i class='fa fa-" + this.categories[category].img
                        + "' category='" + category + "'></i>&nbsp; " + this.categories[category].desc + "</a>").on("click", this.displaySubCategories.bind(this)).appendTo(container);
                }
                c.append(container);
            }
        },

        displaySubCategories: function (e) {
            var category = $(e.target.children[0]).attr("category");
            var c = $("#subCategories");
            c.empty();
            var container = $("<div class='list-group'></div>");
            for (var subcategory in this.categories[category]["subcategories"]) {
                // exists a special key?
                var key = this.categories[category]["subcategories"][subcategory].key ? this.categories[category]["subcategories"][subcategory].key : this.categories[category]["key"];
                $("<a style='border: none' class='list-group-item' key='" + key +"' value='" + this.categories[category]["subcategories"][subcategory].value + "'>"
                    + this.categories[category]["subcategories"][subcategory].desc + "</a>").on("click", this.appendToSearchString.bind(this)).appendTo(container);
            }

            c.append(container);
        },

        appendToSearchString: function (e) {
            var el = $(e.target);
            $("#search_text").tokenfield('createToken', {value:  "@" + el.attr("key") + ":" + el.attr("value"), label:  el.attr("value")});
        },

        categories: {
            food: {
                key: "cuisine",
                desc: "Cuisine",
                img: "cutlery",
                subcategories: {
                    pizza: {value: "pizza", desc: "Pizza"},
                    burger: {value: "burger", desc: "Burger"},
                    kebap: {value: "kebap", desc: "Kebap"},
                    sushi: {value: "sushi", desc: "Sushi"},
                    italian: {value: "italian", desc: "Italian"},
                    chinese: {value: "chinese", desc: "Chinese"},
                    japanese: {value: "japanese", desc: "Japanese"},
                    german: {value: "german", desc: "German"},
                    mexican: {value: "mexican", desc: "Mexican"},
                    greek: {value: "greek", desc: "Greek"}
                }
            },
            amenity: {
                key: "amenity",
                desc: "Amenity",
                img: "home",
                subcategories: {
                    restaurant: {value: "restaurant", desc: "Restaurant"},
                    hotel: {key: "tourism", value: "hotel", desc: "Hotel"},
                    cafe: {value: "cafe", desc: "Café"},
                    pharmacy: {value: "pharmacy", desc: "Pharmacy"},
                    hospital: {value: "hospital", desc: "Hospital"},
                    police: {value: "police", desc: "Police"},
                    wc: {value: "toilets", desc: "Toilets"},
                    atm: {value: "atm", desc: "ATM"},
                    fuel: {value: "fuel", desc: "Fuel"},
                    parking: {value: "parking", desc: "Parking"},
                    bank: {value: "bank", desc: "Bank"}
                }
            },
            shopping: {
                key: "shop",
                desc: "Shopping",
                img: "shopping-cart",
                subcategories: {
                    eat: {value: "supermarket", desc: "Supermarket"},
                    bakery: {value: "bakery", desc: "Bakery"},
                    mall: {value: "mall", desc: "Mall"},
                    clothes: {value: "clothes", desc: "Clothes"},
                    shoes: {value: "shoes", desc: "Shoes"},
                    kiosk: {value: "kiosk", desc: "Kiosk"},
                    butcher: {value: "butcher", desc: "Butcher"}
                }
            },
            transport: {
                key: "public_transport",
                desc: "Transport",
                img: "car",
                subcategories: {
                    airport: {key: "aeroway", value: "taxiway", desc: "Airport"},
                    taxi: {key: "amenity", value: "taxi", desc: "Taxi"},
                    ferry: {key: "amenity", value: "ferry_terminal", desc: "Ferry"},
                    oepnv: {value: "platform", desc: "ÖPNV"}
                }
            },
            leisure: {
                key: "leisure",
                desc: "Leisure",
                img: "futbol-o",
                subcategories: {
                    dance: {value: "dance", desc: "Dance"},
                    playground: {value: "playground", desc: "Playground"},
                    park: {value: "park", desc: "Park"},
                    pitch: {value: "pitch", desc: "Pitch"},
                    water: {value: "swimming_pool", desc: "Water"}
                }
            }
        }
    }
});