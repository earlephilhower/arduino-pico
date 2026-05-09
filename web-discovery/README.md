# UDP discovery page

This standalone project uses React for the UI and a small Node server for the UDP work that browsers cannot perform directly.

## Available scripts

- `npm install` - install dependencies
- `npm run server` - start the Node API on port `3001`
- `npm run dev` - start the Vite client with `/api` proxied to `http://127.0.0.1:3001`
- `npm run build` - build the React client into `dist/`
- `npm run start` - serve the built client and the discovery API from the Node server
- `npm run lint` - lint the project

## Discovery behavior

The API sends one 8-byte UDP broadcast packet:

1. bytes `0-3`: the selected local IPv4 address
2. bytes `4-5`: ASCII `FS`
3. bytes `6-7`: `0x00 0x00`

The first UDP response received within five seconds is decoded as:

- `NAME`: null-terminated string starting at byte `0`
- `SN`: null-terminated string starting at byte `0xbc`

Because the target UDP port was not specified in the request, the UI asks for it before sending the broadcast.
