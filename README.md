# AIRConfig for GlobalTalk

## What is it?

AIRConfig is a utility to write a list of Host IDs to an Apple Internet Router 3.0 config file. It is written in MPW C.

## Why would you want to do this?

For [#MARCHintosh 2024](http://marchintosh.com), a bunch of retro Mac fans starting using some Apple software from the early 90's to connect up all their local retro Mac LANs together over the internet in order to play games and share files and printers, and dubbed it [#GlobalTalk](http://marchintosh.com/globaltalk.html). To do this, the domain name or IP address of every node needs to be manually configured in Apple Internet Router, one-by-one, and manually updated if anything changes.

This was a project to automate that task, so you just give it a text file of addresses and it reformats it and writes it to your Router Config file.

## Screenshot

![airconfig1 0](https://github.com/kalleboo/AIRConfig/assets/304806/994a06db-f6c6-49f6-a028-58f3594f3f29)

## Disclaimer

This is the first Classic Mac application I've written in C. It was a learning experience. It was done over nights and weekends when I had some free time, and the source code is a mess that was written solely to accomplish a goal - it does not have the polish of something I'd do professionally. Do not use this code to learn how to do something the right way. For entertainment purposes only.

## Requirements

* Requires System 7.1 (same as Apple Internet Router). Runs on up to MacOS 9.
* Compiling requires MPW 3.

## Modifications for git

In order to commit the source files to git, the following modifications have been made. These will have to be reveresed if you want to compile this code in MPW.

* Line endings have been converted from CR (Classic Mac) to LF (Unix)
* Resource files (rsrc) have been run through DeRez to create text files
