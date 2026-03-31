import { expect, test } from "@playwright/test";

import { readFile } from "node:fs/promises";
import path from "node:path";

const fixtureDir = path.join(process.cwd(), "webassembly", "tests", "fixtures", "io");

async function readFixture(name: string): Promise<string> {
  return readFile(path.join(fixtureDir, name), "utf8");
}

test("imports structure and grid without runtime errors", async ({ page }) => {
  const consoleErrors: string[] = [];
  const pageErrors: string[] = [];

  page.on("console", (message) => {
    if (message.type() === "error") {
      consoleErrors.push(message.text());
    }
  });
  page.on("pageerror", (error) => {
    pageErrors.push(String(error));
  });

  const structureText = await readFixture("sample_structure.xsf");
  const chgcarText = await readFixture("sample_chgcar.vasp");

  await page.goto("/workbench");
  await expect(page.getByTestId("workbench-root")).toHaveAttribute("data-ready", "true");

  await page.evaluate(
    ({ fileName, text }) => {
      const api = (window as Window & { __VTK_WORKBENCH_TEST__?: {
        importXsfText: (name: string, body: string) => void;
      } }).__VTK_WORKBENCH_TEST__;
      if (!api) {
        throw new Error("Workbench test API is not available");
      }
      api.importXsfText(fileName, text);
    },
    { fileName: "sample_structure.xsf", text: structureText }
  );

  await expect
    .poll(() =>
      page.evaluate(() => {
        const api = (window as Window & { __VTK_WORKBENCH_TEST__?: {
          getStructureCount: () => number;
        } }).__VTK_WORKBENCH_TEST__;
        return api?.getStructureCount() ?? 0;
      })
    )
    .toBeGreaterThan(0);

  const structureId = await page.evaluate(() => {
    const api = (window as Window & { __VTK_WORKBENCH_TEST__?: {
      getCurrentStructureId: () => number;
    } }).__VTK_WORKBENCH_TEST__;
    return api?.getCurrentStructureId() ?? -1;
  });

  await page.evaluate((id) => {
    const api = (window as Window & { __VTK_WORKBENCH_TEST__?: {
      setStructureVisible: (targetId: number, visible: boolean) => void;
    } }).__VTK_WORKBENCH_TEST__;
    api?.setStructureVisible(id, false);
  }, structureId);

  await expect
    .poll(() =>
      page.evaluate((targetId) => {
        const api = (window as Window & { __VTK_WORKBENCH_TEST__?: {
          isStructureVisible: (visibleId: number) => boolean;
        } }).__VTK_WORKBENCH_TEST__;
        return api?.isStructureVisible(targetId) ?? true;
      }, structureId)
    )
    .toBe(false);

  await page.evaluate((id) => {
    const api = (window as Window & { __VTK_WORKBENCH_TEST__?: {
      setStructureVisible: (targetId: number, visible: boolean) => void;
    } }).__VTK_WORKBENCH_TEST__;
    api?.setStructureVisible(id, true);
  }, structureId);

  await expect
    .poll(() =>
      page.evaluate((targetId) => {
        const api = (window as Window & { __VTK_WORKBENCH_TEST__?: {
          isStructureVisible: (visibleId: number) => boolean;
        } }).__VTK_WORKBENCH_TEST__;
        return api?.isStructureVisible(targetId) ?? false;
      }, structureId)
    )
    .toBe(true);

  await page.evaluate(
    ({ fileName, text }) => {
      const api = (window as Window & { __VTK_WORKBENCH_TEST__?: {
        importChgcarText: (name: string, body: string) => void;
      } }).__VTK_WORKBENCH_TEST__;
      api?.importChgcarText(fileName, text);
    },
    { fileName: "sample_chgcar.vasp", text: chgcarText }
  );

  await expect
    .poll(() =>
      page.evaluate(() => {
        const api = (window as Window & { __VTK_WORKBENCH_TEST__?: {
          hasChargeDensity: () => boolean;
        } }).__VTK_WORKBENCH_TEST__;
        return api?.hasChargeDensity() ?? false;
      })
    )
    .toBe(true);

  const relevantConsoleErrors = consoleErrors.filter(
    (message) => message !== "Failed to load resource: the server responded with a status of 404 (Not Found)"
  );

  expect(pageErrors).toEqual([]);
  expect(relevantConsoleErrors).toEqual([]);
});
