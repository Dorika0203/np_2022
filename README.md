# 2022-1 SKKU Network Project

## Introduction

* Random Chatting Server-Client, NS-3 Application  

* Consist of
    * Helper
    * Client
    * Server
    * Example Topology (Using CSMA channel)
    * Modified application wscript



## How to run

1. Install ns-3 (3.29)  

2. Move directory into "ns-3.29/"  

3. Fork this repository

4. Fetch forked repository  
    ```
    ~/ns-3.29 $ git remote add origin [ forked repo ]
    ~/ns-3.29 $ git fetch origin
    ~/ns-3.29 $ git checkout -t origin/main
    ```
5. Run example topology in scratch
    ```
    ~/ns-3.29 $ ./waf --run topology
    ```

