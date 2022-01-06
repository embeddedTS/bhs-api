# Introduction #

This node.js application is designed to run from a embeddedTS [TS-7680 SBC](https://www.embeddedTS.com/products/TS-7680) and serves a REST API for getting Bozeman Hot Springs outside pool temperature data collected from a Honeywell T775M Controller (2 - 10 VDC output) using Honeywell 50021579-001 thermoresistive sensors.

There are two API endpoints:

* GET /temperatures
    * Returns a JSON array with all three pool temperature data objects.
* GET /temperatures/:pool_id
    * Returns JSON object with :pool_id temperature data where :pool_id is between 0 and 2.

Example response from GET /temperatures
```
#!json

[
   {
      "id":0,
      "vdcin":"5151",
      "temperature":"73.4",
      "temperatureUnit":"F",
      "error":null
   },
   {
      "id":1,
      "vdcin":"4915",
      "temperature":"64.9",
      "temperatureUnit":"F",
      "error":null
   },
   {
      "id":2,
      "vdcin":"5143",
      "temperature":"73.1",
      "temperatureUnit":"F",
      "error":null
   }
]
```


The API has been implemented in two languages:  nodejs and Python.  We'll be using the nodejs version.  Python is included because 1.) it's supposed to be faster (not that I noticed) and 2.) for the fun of it.

This diagram shows how the TS-7680 is connected to the three Honeywell T775M controllers.

![bozeman-hot-springs-ts-7680-honeywell-2_bb-1024.png](https://bitbucket.org/repo/eGL68B/images/713005390-bozeman-hot-springs-ts-7680-honeywell-2_bb-1024.png)

**Note:**  *The code behind expects the Honeywell controllers to be setup using MOD2 set to use SENSOR1 with parameters `TYPE=2-10VDC`, `SETPOINT=100`, `THROTTLING_RANGE=40`, `INTEGRAL=0` so that at 80F, the Vout will be 10 VDC and at 120F, Vout will be 2 VDC.*

**Note:**  *The TS-7680 was connected as such: Pool A (Pump 1) = AN0, Pool B (Pump 2) = AN1, Pool C (Pump 3) = AN2.*

## Bozeman Hot Springs API Installation Instructions ##

Let's pull in all the source code required to run this server.  This involves prepping the system by installing required packages, downloading the code, compiling getadc program, preparing nodejs modules, and finally running a test server session.

1. Start out by installing nodejs, npm, and git
    * `sudo apt-get update && sudo apt-get install nodejs npm git`

1. Clone the GIT repository containing all nodejs files and getadc program
    * `git clone git@bitbucket.org:dhildreth/bhs-api.git bhs-api`

1. Change into the newly created directory (should be in $HOME directory)
    * `cd bhs-api`

1. Compile the getadc program, which our server will use to grab VDC values from the ADC channels of the TS-7680.  Running `make` will compile and install the binary into */usr/local/bin*.
    * `cd src/getadc`
    * `sudo make`
    * `cd -`

1. Run npm to install all dependencies of the API server.
    * `npm install`

1. For debugging purposes, you can now run the server.
    * `node server.js`

Then, use curl or browse to http://localhost:8080/temperatures to get a list of all pool temperatures or http://localhost:8080/temperatures/:pool to get a specific pool matching id :pool.  

## Server Setup ##

These are the instructions for node.js deployment.  First, we'll install `pm2` and get our node.js server running.  Then, we'll need to modify the auto generated systemctl file so it will work without failure.

### Install PM2 ###

1. Install pm2 globally using npm
    * `npm install -g pm2`

1. Start the server using pm2, naming it bhs-api
    * `pm2 start /home/bhs/bhs-api/nodejs/server.js --name bhs-api`

1. Set our bhs-api server to run when the system powers on.  Running this command will generate another command required to run as sudo (see next step).
    * `pm2 startup`

1. Configure systemd to start this application when the system boots up.  **You will get a failure here!**  Please see the next steps to correct this.
    * `sudo env PATH=$PATH:/usr/bin /usr/local/lib/node_modules/pm2/bin/pm2 startup systemd -u bhs --hp /home/bhs`

1. Save the pm2 configuration.
    * `pm2 save`

1. Fix issues related to systemctl.  We need to modify the auto generated */etc/systemd/system/pm2-bhs.service* file.  First, we need to change the service type from `forked` to `simple`.  Then, since the TS-7680 is a little slower than pm2 defaults are set to, we'll also set the kill-timeout option to be 60 seconds (default 8 seconds).

    * `sudo vim /etc/systemd/system/pm2-bhs.service`

```
#!shell

[Unit]
Description=PM2 process manager
Documentation=https://pm2.keymetrics.io/
After=network.target

[Service]
Type=simple
User=bhs
LimitNOFILE=infinity
LimitNPROC=infinity
LimitCORE=infinity
TimeoutStartSec=8
Environment=PATH=/usr/bin:/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin
Environment=PM2_HOME=/home/bhs/.pm2
PIDFile=/home/bhs/.pm2/pm2.pid

ExecStart=/usr/local/lib/node_modules/pm2/bin/pm2 --kill-timeout 60 resurrect
ExecReload=/usr/local/lib/node_modules/pm2/bin/pm2 --kill-timeout 60  reload all
ExecStop=/usr/local/lib/node_modules/pm2/bin/pm2 --kill-timeout 60  kill

[Install]
WantedBy=multi-user.target
```

1. Reload the systemctl config files.
    * `sudo systemctl daemon-reload`

1. Start the server.  You should not receive any errors and you should be able to use the API.  Should you encounter any errors, you can always run `sudo systemctl status -l pm2-bhs`.
    * `sudo systemctl start pm2-bhs`


## Example Webpage ##

Also included in this repository is an example webpage which consumes the API and displays current pool temperature readings in near real-time (every 15 seconds).  It uses AJAX and jQuery to fetch the data and update HTML elements with data.

![Screen Shot 2017-03-03 at 12.18.25 PM.png](https://bitbucket.org/repo/eGL68B/images/2555406468-Screen%20Shot%202017-03-03%20at%2012.18.25%20PM.png)



## Who do I talk to? ##

* Derek Hildreth - eBusiness Manager - embeddedTS
* Andy Barham - Manager - Bozeman Hot Springs