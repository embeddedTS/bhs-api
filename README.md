# Introduction #

This node.js application is designed to run from a Technologic Systems [TS-7680 SBC](https://www.embeddedarm.com/products/TS-7680) and serves a REST API for getting Bozeman Hot Springs outside pool temperature data collected from a Honeywell T775 Controller (2 - 10 VDC output).


![bozeman-hot-springs-ts-7680-honeywell_bb-1024.png](https://bitbucket.org/repo/eGL68B/images/925926239-bozeman-hot-springs-ts-7680-honeywell_bb-1024.png)

## Installation Instructions ##

Designed to be run on a [TS-7680 SBC](https://www.embeddedarm.com/products/TS-7680).

* apt-get update && apt-get install nodejs npm git
* git clone git@bitbucket.org:dhildreth/bozeman-hot-springs-pool-temperatures.git bhs-api
* cd bhs-api
* npm install
* node server.js

Then, use curl or browse to http://localhost:8080/temperatures to get a list of all pool temperatures or http://localhost:8080/temperatures/:pool to get a specific pool matching id :pool.  


## Who do I talk to? ##

* Derek Hildreth - eBusiness Manager - Technologic Systems
* Andy Barham - Manager - Bozeman Hot Springs


## Example Webpage ##

![Screen Shot 2017-03-03 at 12.18.25 PM.png](https://bitbucket.org/repo/eGL68B/images/2555406468-Screen%20Shot%202017-03-03%20at%2012.18.25%20PM.png)