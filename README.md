# nixia
nixia is traffic geneeration client tool to test servers and network
It is still work in progress and it is designed to be a Client Framework which will allow us to test multiple protocols.
Currently, it supports HTTP.

# Requires
Cmake >= 2.8

# Build

     make;
     sudo make install

# To run test
Runs Valgrind and Regression suite.  

         make test
*TBD*: Unit test suite

# Run
Nixia requires a json file as input.

         nixia -f config.json
  
# Input config

An example of a input config json

     {
        "main": {
            "name": "output_10000c1000cu100000r",
            "protocols": ["http"],
        },
    
        "http": {
            "url": "127.0.0.1",
            "url_path": "",
            "total_connections": 10000,
            "concurrency": 1000,
            "connections_per_second": "",
            "total_requests": 100000,
            "requests_per_second": "",
        }
     }

You can find more examples under examples/configurations directory
