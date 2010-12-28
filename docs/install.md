# WriteOnWall Installation

The main component of the WriteOnWall Installation is the Node.js based server. It receives tag events from the RFID readers, active and idle state from the OpenFrameworks video software, can send commands to OpenFrameworks and can talk with Anymeta. It also drives the UI, by means of a persistent Web Socket connection to a transparent web view overlay (in other words, the UI is small web page).

Let's work through these different parts from the outside in. We'll assume the main server is available at `wowserver:8080`.

## RFID Readers

The installation requires three RFID readers, corresponding to the "props", "claim" and "clear" actions. The readers make HTTP calls to the `wowserver`. The RFID tag value is always the message body. Use `POST` when a tag is added, `DELETE` when it's removed. The action identifier is part of the request URL, with `like` being used for "props", `made` for "claim" and `clear` for "clear".

Example:

    POST wowserver:8080/tag/like
    
    (with the RFID tag value being the POST body)

Currently the server only cares about add events.

At the Mediamatic setup we use an EEE PC which is hooked to the three RFID readers. The hostname `wowserver` is set to the IP address of `mediamatic-12.local`, which is the MacBook Pro on which the server is running. You need to start a reader process for each connected reader device. The process requires `fizzjik` and `twisted` to be installed, it's a modified version of the `sonmicro` reader script.

To start the reader:

    python reader/sonmicro.py --id=like --device=/dev/ttyUSB0

Make sure the specified `id` is correct for the `device` referenced.

It's not necessary for a separate device (from the MacBook) to be used, as long as the RFID readers work fine over USB and can make the HTTP requests. Just ensure that the `wowserver` hostname is configured correctly.

## OpenFrameworks

The OpenFrameworks app drives the light tracking software. It talks to the main server via a TPC connection to port 8081 on the local host. This means the main server must run on the same machine as the OpenFrameworks app. It will automatically connect (and reconnect), so you just need to make sure it's running and positioned correctly.

TODO: FURTHER DOCUMENTATION. DIRK?
* How to configure outputdir? Is it a default?

## UI Layer

The UI layer is a small web page that runs in a specially developed TransparentWebView app. It's to be positioned over the OpenFrameworks output. When you open the app, simply load the location `http://wowserver:8080` (assuming the `wowserver` hostname points to the machine running the main server; if it's the same machine `http://localhost:8080` will of course also work).

The UI is a grid of two rows and three columns. Each column corresponds to a reader. UI feedback appears in the bottom row, the top row simply shows "Write On Wall". The blocks in the grid should line up to each other or window frames that might divide them. You can load `http://wowserver:8080?calibrate` to go into debug mode. This colors the blocks black and the space between them white. Edit the bottom part of `web/style.css` to resize and position each block. Note that the `left` and `right` classes are defined as looking from the projector to the projection.

When not in calibration mode, black bars are overlayed on the gaps between the different blocks. This should decrease the reflection on window frames back into the camera.

## Main Server

The main server runs Node.js; it requires v0.2.5. Also make sure to install NPM, the Node Package Manager, and then run the following commands to install dependencies:

    npm install https://github.com/kriszyp/promised-io/tarball/c53b75064376dc2ef8181b4949bb03fe28b93c63
    npm install iniparser@1.0.1
    npm install node-static@0.5.2
    npm install socket.io@0.6.3
    npm install watch@0.2.1
    npm install oauth@0.8.4

The server script looks for configuration in the `$HOME/.anymeta` file. You need to add one field to that config, namely the `outputdir` where OpenFrameworks will save screen captures to. An example config:

    [wow]
    entrypoint = http://www.mediamatic.net/services/rest/
    comment = 
    c_key = sc-1234
    c_sec = 
    t_key = 1234
    t_sec = 1234
    outputdir = /Users/sysop/Desktop/wowData

You can then start the server:

    node server/wow.js wow

However it's best to set up the LaunchDaemon for this, as it'll make sure the server restarts if it crashes. Edit `net.mediamatic.WriteOnWall.plist` to make sure the paths are correct. Then run `launchctl load -w net.mediamatic.WriteOnWall.plist` to start the server. To restart the server, use `launchctl stop net.mediamatic.WriteOnWall`. To stop the server use `launchctl unload -w net.mediamatic.WriteOnWall.plist`. Note that when you use LaunchDaemon, the logs go straight to Console app.

## Debugging

Always make sure the following holds:

* The machine running the readers has the hostname `wowserver` configured with the IP of the machine running the main server
* The readers are running and configured correctly
* The OpenFrameworks code is running and positioned correctly
* The TransparentWebView is running, has loaded the correct page, and is positioned correctly
* The main server is running, no errors
* The PS3 camera is connected and working
* The Anymeta access keys are correct
* The Anymeta entrypoint is reachable

It's possible for each of the components to become stuck. Restart or reload as appropriate.

You can trigger the idle state by opening a telnet connection to `wowserver 8081`. Type `idle` (and hit return) to enable the idle state, type `active` to disable it. This is great for checking what the UI looks like without having to wait for the idle state to be triggered.
