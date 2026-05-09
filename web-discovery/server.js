import dgram from 'node:dgram'
import { createServer } from 'node:http'
import { existsSync, createReadStream } from 'node:fs'
import { stat } from 'node:fs/promises'
import os from 'node:os'
import { dirname, extname, join, normalize } from 'node:path'
import { fileURLToPath } from 'node:url'

const HOST = process.env.HOST ?? '0.0.0.0'
const PORT = Number(process.env.PORT ?? 3001)
const BROADCAST_TIMEOUT_MS = 5_000
const DEFAULT_BROADCAST_ADDRESS = '255.255.255.255'
const DISCOVERY_PORT = 48899
const SERIAL_NUMBER_OFFSET = 0xbc

const __dirname = dirname(fileURLToPath(import.meta.url))
const distDir = join(__dirname, 'dist')
const mimeTypes = {
  '.css': 'text/css; charset=utf-8',
  '.html': 'text/html; charset=utf-8',
  '.js': 'text/javascript; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.svg': 'image/svg+xml',
}

function getIPv4Addresses() {
  const interfaces = os.networkInterfaces()
  const addresses = new Set()

  Object.values(interfaces).forEach((entries) => {
    entries?.forEach((entry) => {
      if (entry.family === 'IPv4' && !entry.internal) {
        addresses.add(entry.address)
      }
    })
  })

  return [...addresses].sort()
}

function isValidIPv4(value) {
  const octets = value.split('.')

  return octets.length === 4 && octets.every((octet) => {
    const number = Number(octet)
    return /^\d+$/.test(octet) && number >= 0 && number <= 255
  })
}

function encodeDiscoveryPacket(serverIp) {
  const octets = serverIp.split('.').map(Number)
  return Buffer.from([...octets, 0x46, 0x53, 0x00, 0x00])
}

function decodeCString(buffer, startOffset) {
  let endOffset = startOffset

  while (endOffset < buffer.length && buffer[endOffset] !== 0) {
    endOffset += 1
  }

  return buffer.toString('utf8', startOffset, endOffset)
}

function sendJson(response, statusCode, payload) {
  response.writeHead(statusCode, { 'Content-Type': 'application/json; charset=utf-8' })
  response.end(JSON.stringify(payload))
}

async function serveStatic(requestPath, response) {
  const safePath = normalize(requestPath)
    .replace(/^([.][.][/\\])+/, '')
    .replace(/^\/+/, '')
  const targetPath = safePath === '' ? 'index.html' : safePath
  const candidatePath = join(distDir, targetPath)

  if (!existsSync(distDir)) {
    sendJson(response, 503, {
      error: 'The client bundle is not built yet. Run npm run build before npm start.',
    })
    return
  }

  try {
    const fileStats = await stat(candidatePath)
    if (!fileStats.isFile()) {
      throw new Error('Not a file')
    }

    response.writeHead(200, {
      'Content-Type': mimeTypes[extname(candidatePath)] ?? 'application/octet-stream',
    })
    createReadStream(candidatePath).pipe(response)
    return
  } catch {
    const fallbackPath = join(distDir, 'index.html')
    response.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' })
    createReadStream(fallbackPath).pipe(response)
  }
}

function discoverDevice({ serverIp, port, broadcastAddress }) {
  return new Promise((resolve, reject) => {
    const socket = dgram.createSocket('udp4')
    const packet = encodeDiscoveryPacket(serverIp)
    let settled = false

    const finish = (callback) => (value) => {
      if (settled) {
        return
      }

      settled = true
      clearTimeout(timeout)
      socket.close(() => callback(value))
    }

    const resolveOnce = finish(resolve)
    const rejectOnce = finish(reject)

    const timeout = setTimeout(() => {
      rejectOnce(new Error('Timed out waiting for a UDP response.'))
    }, BROADCAST_TIMEOUT_MS)

    socket.once('error', rejectOnce)
    socket.once('message', (message, remote) => {
      resolveOnce({
        name: decodeCString(message, 0),
        serialNumber: decodeCString(message, SERIAL_NUMBER_OFFSET),
        responderAddress: remote.address,
        responderPort: remote.port,
      })
    })

    socket.bind(0, serverIp, () => {
      socket.setBroadcast(true)
      socket.send(packet, port, broadcastAddress, (error) => {
        if (error) {
          rejectOnce(error)
        }
      })
    })
  })
}

async function parseJsonBody(request) {
  const chunks = []

  for await (const chunk of request) {
    chunks.push(chunk)
  }

  if (chunks.length === 0) {
    return {}
  }

  return JSON.parse(Buffer.concat(chunks).toString('utf8'))
}

const server = createServer(async (request, response) => {
  if (!request.url) {
    sendJson(response, 400, { error: 'Missing request URL.' })
    return
  }

  const url = new URL(request.url, `http://${request.headers.host ?? 'localhost'}`)

  if (request.method === 'GET' && url.pathname === '/api/interfaces') {
    sendJson(response, 200, {
      interfaces: getIPv4Addresses(),
      defaultBroadcastAddress: DEFAULT_BROADCAST_ADDRESS,
      timeoutMs: BROADCAST_TIMEOUT_MS,
    })
    return
  }

  if (request.method === 'POST' && url.pathname === '/api/discover') {
    try {
      const body = await parseJsonBody(request)
      const serverIp = body.serverIp
      const broadcastAddress = body.broadcastAddress ?? DEFAULT_BROADCAST_ADDRESS
      const interfaces = getIPv4Addresses()

      if (!interfaces.includes(serverIp)) {
        sendJson(response, 400, { error: 'Select a valid local IPv4 address.' })
        return
      }

      if (!isValidIPv4(broadcastAddress)) {
        sendJson(response, 400, { error: 'Enter a valid IPv4 broadcast address.' })
        return
      }

      const result = await discoverDevice({ serverIp, port: DISCOVERY_PORT, broadcastAddress })
      sendJson(response, 200, result)
    } catch (error) {
      const message = error instanceof SyntaxError
        ? 'Request body must be valid JSON.'
        : error instanceof Error
          ? error.message
          : 'UDP discovery failed.'
      const statusCode = message.includes('Timed out') ? 504 : 500
      sendJson(response, statusCode, { error: message })
    }
    return
  }

  await serveStatic(url.pathname, response)
})

server.listen(PORT, HOST, () => {
  console.log(`UDP discovery server listening on http://${HOST}:${PORT}`)
})
