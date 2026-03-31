import { defineConfig } from "@playwright/test";

export default defineConfig({
  testDir: "./tests/e2e",
  timeout: 120_000,
  expect: {
    timeout: 30_000,
  },
  use: {
    baseURL: "http://127.0.0.1:3000",
    headless: true,
  },
  webServer: {
    command: "npm run dev -- --hostname 127.0.0.1 --port 3000",
    url: "http://127.0.0.1:3000/workbench",
    reuseExistingServer: true,
    timeout: 180_000,
  },
  projects: [
    {
      name: "msedge",
      use: {
        browserName: "chromium",
        channel: "msedge",
      },
    },
  ],
});
