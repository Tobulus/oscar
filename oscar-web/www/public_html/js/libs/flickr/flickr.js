define(["jquery", "jqueryui", "slimbox", "conf"], function ($, jqueryui, slimbox, config) {
    return {
        _api_key: "46f7a6dce46471b81aa7c1592fcc9733",

        _query: function (text, geopos, callback) {
            var urls = [];
            var service = 'https://api.flickr.com/services/rest/?method=flickr.photos.search'
            service += '&api_key=' + this._api_key
            service += '&text=' + text
            service += '&format=json'
            service += '&nojsoncallback=1'
            service += '&sort=relevance';
            service += '&lat=' + geopos.lat
            service += '&lon=' + geopos.lng;

            $.getJSON(service, function (data) {
                var photo;
                for (var i in data.photos.photo) {
                    photo = data.photos.photo[i];
                    urls.push({
                        url: 'https://farm' + photo.farm + '.staticflickr.com/' + photo.server + '/' + photo.id + '_' + photo.secret + '_t.jpg',
                        title: photo.title,
                        photo_id: photo.id
                    });
                }
                callback(urls);
            });
        },

        getImagesForLocation: function (name, geopos) {
            this._query(name, geopos, this._showImagesForLocation.bind(this));
        },

        _setPhotoUrl: function (photo_id, el) {
            var service = "https://api.flickr.com/services/rest/?method=flickr.photos.getSizes"
            service += "&format=json"
            service += "&nojsoncallback=1"
            service += "&api_key=" + this._api_key
            service += "&photo_id=" + photo_id;

            $.getJSON(service, function (data) {
                var i = data.sizes.size.length - 1;

                while(data.sizes.size[i].width > 1024){
                    i--;
                }

                $(el).attr("href", data.sizes.size[i].source);
            });
        },

        _showImagesForLocation: function (urls) {
            var flickr_img = $('#flickr_images').empty();

            for (var i = 0; i < 50 && i < urls.length; i++) {
                var obj = urls[i];
                var link = $("<a />").attr("rel", "lightbox-group").attr("title", obj.title || "");
                $("<img/>").attr("src", obj.url).appendTo(link);
                this._setPhotoUrl(urls[i].photo_id, link);
                link.appendTo(flickr_img);
            }

            if (urls.length > 0) {
                $('#flickr').show("slide", {direction: "right"}, config.styles.slide.speed);
                // slimbox fails with dynamic content => updated added images
                $("a[rel^='lightbox']").slimbox({}, null, function (el) {
                    return (this == el) || ((this.rel.length > 8) && (this.rel == el.rel));
                });
            } else {
                $('#flickr').hide("slide", {direction: "right"}, config.styles.slide.speed);
            }
        }
    }
});