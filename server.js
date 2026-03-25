// server.js (ESM, monkey-patch 방식)

import http from 'http'
import { createRequire } from 'module'

const require = createRequire(import.meta.url)

// 🔴 http.createServer 패치
const originalCreateServer = http.createServer

http.createServer = function (requestListener) {
  return originalCreateServer.call(http, (req, res) => {
    // 🔴 SharedArrayBuffer 필수 헤더
    res.setHeader('Cross-Origin-Opener-Policy', 'same-origin')
    res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp')
    res.setHeader('Cross-Origin-Resource-Policy', 'same-origin')

    return requestListener(req, res)
  })
}

// 🔵 이 줄에서 Next.js standalone 서버가 실행됨
require('./original-server.js')

// ❗ 절대 listen() 호출하지 말 것

