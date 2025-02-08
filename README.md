# Resonate

Resonate synchronizes Philips Hue lights with video using predefined algorithms. Users
must manually start Resonate at the same time as the video.

## Credentials

The following credentials are required to use the Philips Hue Entertainment API.  Export
the environment variables in `~/.bashrc` or `~/.zprofile`.

### Username

* The `username` returned by the `/api` endpoint.
* The `hue-application-key` passed in the header of each API request.
* The `HUE_USERNAME` environment variable.

### Client Key

* The `clientkey` returned by the `/api` endpoint when `generateclientkey` is `true`.
* The pre-shared key (PSK) used to encrypt the DTLS connection between the client and
  Hue bridge.
* The `HUE_CLIENTKEY` environment variable.

### Application ID

* The `hue-application-id` returned by the `/auth/v1` endpoint.
* The PSK identity used to setup the DTLS connection.
* The `HUE_APPLICATION_ID` environment variable.

### Getting new credentials

1. Revoke access for any old apps on the Philips Hue website.

2. Find the Hue Bridge IP address in the app.

3. `POST` the following to `https://<Hue bridge IP address>/api`:

   ```json
   { "devicetype": "Resonate#MacBook", "generateclientkey": true }
   ```

4. `GET` `https://<Hue bridge IP address>/auth/v1`.

## Build

```
mkdir build && cd build
cmake ..
make
```

## Run

```
./resonate <Hue bridge IP address>
```
