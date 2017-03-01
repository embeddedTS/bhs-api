var restify = require('restify');
var Promise = require('promise');

var exec = require('child_process').exec;

var Pool = function (id) {
    this.id = id;
    this.vdcin = null;
    this.temperature = null;
    this.temperatureUnit = "F"; // Only unit supported at this time
}

Pool.prototype.init = function(callback) {
    var _this = this;

    var promise = new Promise(function(resolve, reject) {
        var cmd = 'getadc ' + _this.id;

        exec(cmd, function(error, stdout, stderr) {
            if(!error) {
                resolve(stdout);
            }
            else {
                reject(error);
            }
        });
    });

    promise.then(function(response) {
        _this.vdcin = response;
        _this.setTemperature(_this.vdcin);
        callback(_this);
    })
    .catch(function(error) {
        callback(error);
    });
};

Pool.prototype.setTemperature = function () {
    var _this = this;

    // TODO: Conversion from VDC to F ( or maybe this is done in getadc? )
    var convertedTemperature = _this.vdcin / 1000;
    _this.temperature = convertedTemperature;
};

Pool.prototype.getJson = function() {
    var _this = this;
    return JSON.stringify(_this);
};


var server = restify.createServer();
server.pre(restify.pre.userAgentConnection());

/*******************************************************************************
* GET /temperatures
*   - Returns array of json objects containing all pool temperatures
*******************************************************************************/
server.get('/temperatures', function(req, res, next) {

  res.send("NYI");
  next();

});

/*******************************************************************************
* GET /temperatures/:pool
*   - Returns json object containing pool id and temperature
*******************************************************************************/
server.get('/temperatures/:pool', function(req, res, next) {

    var pool = new Pool(req.params.pool);

    pool.init(function(response) {

	// If we didn't return an object, it must mean it's an error message.
        if (typeof response === 'object') {
            res.json(response);
        }
        else 
            res.json({'error': response});

        next();
    });
});

server.listen(8080, function() {
  console.log('%s listening at %s', server.name, server.url);
});
