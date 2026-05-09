import { useEffect, useMemo, useState } from 'react'
import './App.css'

const initialResult = {
  name: '',
  responderAddress: '',
  responderPort: '',
  serialNumber: '',
}

function App() {
  const [interfaces, setInterfaces] = useState([])
  const [serverIp, setServerIp] = useState('')
  const [broadcastAddress, setBroadcastAddress] = useState('255.255.255.255')
  const [port, setPort] = useState('')
  const [result, setResult] = useState(initialResult)
  const [error, setError] = useState('')
  const [status, setStatus] = useState('Loading local network interfaces…')
  const [isDiscovering, setIsDiscovering] = useState(false)

  useEffect(() => {
    const loadInterfaces = async () => {
      try {
        const response = await fetch('/api/interfaces')
        const payload = await response.json()

        if (!response.ok) {
          throw new Error(payload.error ?? 'Unable to load local network interfaces.')
        }

        setInterfaces(payload.interfaces)
        setBroadcastAddress(payload.defaultBroadcastAddress)
        setServerIp(payload.interfaces[0] ?? '')
        setStatus(
          payload.interfaces.length > 0
            ? `Ready to broadcast. Listening for up to ${payload.timeoutMs / 1000} seconds.`
            : 'No non-loopback IPv4 addresses were detected on the server.',
        )
      } catch (loadError) {
        setError(loadError instanceof Error ? loadError.message : 'Unable to load local network interfaces.')
        setStatus('Unable to prepare UDP discovery.')
      }
    }

    loadInterfaces()
  }, [])

  const canDiscover = useMemo(
    () => Boolean(serverIp) && Boolean(port) && !isDiscovering,
    [isDiscovering, port, serverIp],
  )

  const handleDiscover = async (event) => {
    event.preventDefault()
    setIsDiscovering(true)
    setError('')
    setResult(initialResult)
    setStatus('Sending the 8-byte broadcast packet and waiting for a response…')

    try {
      const response = await fetch('/api/discover', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          broadcastAddress,
          port: Number(port),
          serverIp,
        }),
      })
      const payload = await response.json()

      if (!response.ok) {
        throw new Error(payload.error ?? 'UDP discovery failed.')
      }

      setResult(payload)
      setStatus(`Response received from ${payload.responderAddress}:${payload.responderPort}.`)
    } catch (discoverError) {
      setError(discoverError instanceof Error ? discoverError.message : 'UDP discovery failed.')
      setStatus('No device information available.')
    } finally {
      setIsDiscovering(false)
    }
  }

  return (
    <main className="page-shell">
      <section className="hero-card">
        <p className="eyebrow">Standalone React project</p>
        <h1>UDP discovery page</h1>
        <p className="description">
          The browser UI asks the local Node server to broadcast an 8-byte UDP packet made from the
          selected server IP, the ASCII characters <code>FS</code>, and two zero bytes. The first
          response received within five seconds is decoded into <strong>NAME</strong> and
          <strong> SN</strong>.
        </p>
      </section>

      <section className="content-grid">
        <form className="panel" onSubmit={handleDiscover}>
          <div className="panel-header">
            <h2>Discovery request</h2>
            <p>{status}</p>
          </div>

          <label>
            <span>Server IPv4 address</span>
            <select value={serverIp} onChange={(event) => setServerIp(event.target.value)}>
              {interfaces.length === 0 ? (
                <option value="">No IPv4 addresses found</option>
              ) : (
                interfaces.map((address) => (
                  <option key={address} value={address}>
                    {address}
                  </option>
                ))
              )}
            </select>
          </label>

          <label>
            <span>Broadcast address</span>
            <input
              type="text"
              inputMode="numeric"
              value={broadcastAddress}
              onChange={(event) => setBroadcastAddress(event.target.value)}
              placeholder="255.255.255.255"
            />
          </label>

          <label>
            <span>UDP port</span>
            <input
              type="number"
              min="1"
              max="65535"
              value={port}
              onChange={(event) => setPort(event.target.value)}
              placeholder="Enter the target UDP port"
            />
          </label>

          <button type="submit" disabled={!canDiscover}>
            {isDiscovering ? 'Waiting for response…' : 'Send broadcast packet'}
          </button>
        </form>

        <section className="panel result-panel" aria-live="polite">
          <div className="panel-header">
            <h2>Device response</h2>
            <p>Decoded from the first UDP packet returned within the timeout window.</p>
          </div>

          <dl>
            <div>
              <dt>NAME</dt>
              <dd>{result.name || '—'}</dd>
            </div>
            <div>
              <dt>SN</dt>
              <dd>{result.serialNumber || '—'}</dd>
            </div>
            <div>
              <dt>Responder</dt>
              <dd>
                {result.responderAddress && result.responderPort
                  ? `${result.responderAddress}:${result.responderPort}`
                  : '—'}
              </dd>
            </div>
          </dl>

          {error ? <p className="error-banner">{error}</p> : null}
        </section>
      </section>
    </main>
  )
}

export default App
