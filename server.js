var restify = require('restify');
var Promise = require('promise');

var exec = require('child_process').exec;

var Pool = function (id) {
    this.id = id;
    this.vdcin = null;
    this.temperature = null;
    this.temperatureUnit = "F"; // Only unit supported at this time
    this.error = null;
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
        _this.temperature = null;
        _this.temperatureUnit = null;
        _this.error = error;
        callback(_this);
    });
};

Pool.prototype.setTemperature = function () {
    var _this = this;

    /***
    * y = m * x + b       vdc = m * temp + b
    * x = (y - b) / m     temp = (vdc - b) / m
    *
    * Assuming 2000 mV is -40 and 10000 mV is 248 (max temperature range of T775M):
    * temp = (vdc - 3111.11111111) / 27.7777777778
    * 6000 mV ~= 103.9 F
    ***/

    var m = 27.778;
    var b = 3111.111;

    var convertedTemperature = (_this.vdcin - b) / m;
    _this.temperature = convertedTemperature.toFixed(1);
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

    numberOfPools = 3;
    currentPool = 0;
    allPools = [];

    function get_pool() {

        // We're done going through pools at this point, so return!
        if (currentPool >= numberOfPools) {
            //console.log("allPools: ", allPools);
            res.setHeader("Access-Control-Allow-Origin", "*");
            res.json(allPools);
            next();
        }
        else {
            var pool = new Pool(currentPool);

            pool.init(function(response) {
                allPools.push(response);
                currentPool++; 
                get_pool();
            });
        }
    }

    get_pool();
});

/*******************************************************************************
* GET /temperatures/:pool
*   - Returns json object containing pool id and temperature
*******************************************************************************/
server.get('/temperatures/:pool', function(req, res, next) {

    var pool = new Pool(req.params.pool);

    pool.init(function(response) {
        res.setHeader("Access-Control-Allow-Origin", "*");
        res.json(response);
        next();
    });
});

server.listen(8080, function() {
    console.log('%s listening at %s', server.name, server.url);
});
