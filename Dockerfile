# Stage 1: Node.js Build
FROM node:22-alpine AS node-builder

WORKDIR /app

COPY package*.json ./
RUN npm install && npm cache clean --force

COPY app/ ./app/
COPY components/ ./components/
COPY lib/ ./lib/
COPY public/ ./public/
COPY types/ ./types/
COPY *.json *.js *.ts *.mjs ./

RUN echo "=== WASM files ===" && \
    ls -la public/wasm/ && \
    test -f public/wasm/VTK-Workbench.wasm || (echo "ERROR: WASM not found!" && exit 1)

RUN npm run build

# Stage 2: Production Runtime
FROM node:22-alpine AS runtime

RUN apk add --no-cache dumb-init wget

RUN addgroup -g 1001 -S nodejs && \
    adduser -S nextjs -u 1001

WORKDIR /app

# standalone 복사
COPY --from=node-builder --chown=nextjs:nodejs /app/.next/standalone ./
COPY --from=node-builder --chown=nextjs:nodejs /app/.next/static ./.next/static
COPY --from=node-builder --chown=nextjs:nodejs /app/public ./public

# Next.js 원본 server.js 이름 변경
RUN mv ./server.js ./original-server.js

# 커스텀 wrapper server.js 복사
COPY --chown=nextjs:nodejs server.js ./server.js

USER nextjs

EXPOSE 3000
ENV PORT=3000
ENV NODE_ENV=production

HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD wget --quiet --tries=1 --spider http://localhost:3000/api/health || exit 1

CMD ["dumb-init", "node", "server.js"]