import { expect, test, type Page } from "@playwright/test";

type ShellLayoutPresetCode = 0 | 1 | 2 | 3 | 4;
type ShellWindowIdCode = 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11;
type EditorPanelCode = 0 | 1 | 2;
type BuilderPanelCode = 0 | 1 | 2;
type DataPanelCode = 0 | 1 | 2 | 3;

type ShellStateSnapshot = {
  pendingLayoutPreset: ShellLayoutPresetCode;
  hasPendingEditorRequest: boolean;
  hasPendingBuilderRequest: boolean;
  hasPendingDataRequest: boolean;
};

type WorkbenchTestApi = {
  requestLayoutPreset: (preset: ShellLayoutPresetCode) => void;
  openEditorPanel: (request: EditorPanelCode) => void;
  openBuilderPanel: (request: BuilderPanelCode) => void;
  openDataPanel: (request: DataPanelCode) => void;
  isShellWindowVisible: (windowId: ShellWindowIdCode) => boolean;
  getShellStateSnapshot: () => ShellStateSnapshot;
};

const LAYOUT = {
  NONE: 0,
  LAYOUT_1: 1,
  LAYOUT_2: 2,
  LAYOUT_3: 3,
  RESET: 4,
} as const;

const WINDOW = {
  VIEWER: 0,
  MODEL_TREE: 1,
  PERIODIC_TABLE: 4,
  CRYSTAL_TEMPLATES: 5,
  BRILLOUIN_ZONE_PLOT: 6,
  CREATED_ATOMS: 7,
  BONDS_MANAGEMENT: 8,
  CELL_INFORMATION: 9,
  CHARGE_DENSITY_VIEWER: 10,
  SLICE_VIEWER: 11,
} as const;

const EDITOR = {
  ATOMS: 0,
  BONDS: 1,
  CELL: 2,
} as const;

const BUILDER = {
  ADD_ATOMS: 0,
  BRAVAIS_LATTICE_TEMPLATES: 1,
  BRILLOUIN_ZONE: 2,
} as const;

const DATA = {
  ISOSURFACE: 0,
  SURFACE: 1,
  VOLUMETRIC: 2,
  PLANE: 3,
} as const;

async function getApiSnapshot(page: Page): Promise<ShellStateSnapshot> {
  return page.evaluate(() => {
    const api = (window as Window & { __VTK_WORKBENCH_TEST__?: WorkbenchTestApi }).__VTK_WORKBENCH_TEST__;
    if (!api) {
      throw new Error("Workbench test API is not available");
    }
    return api.getShellStateSnapshot();
  });
}

async function requestLayoutPreset(
  page: Page,
  preset: ShellLayoutPresetCode
) {
  await page.evaluate((value) => {
    const api = (window as Window & { __VTK_WORKBENCH_TEST__?: WorkbenchTestApi }).__VTK_WORKBENCH_TEST__;
    if (!api) {
      throw new Error("Workbench test API is not available");
    }
    api.requestLayoutPreset(value);
  }, preset);
}

async function openEditorPanel(
  page: Page,
  request: EditorPanelCode
) {
  await page.evaluate((value) => {
    const api = (window as Window & { __VTK_WORKBENCH_TEST__?: WorkbenchTestApi }).__VTK_WORKBENCH_TEST__;
    if (!api) {
      throw new Error("Workbench test API is not available");
    }
    api.openEditorPanel(value);
  }, request);
}

async function openBuilderPanel(
  page: Page,
  request: BuilderPanelCode
) {
  await page.evaluate((value) => {
    const api = (window as Window & { __VTK_WORKBENCH_TEST__?: WorkbenchTestApi }).__VTK_WORKBENCH_TEST__;
    if (!api) {
      throw new Error("Workbench test API is not available");
    }
    api.openBuilderPanel(value);
  }, request);
}

async function openDataPanel(
  page: Page,
  request: DataPanelCode
) {
  await page.evaluate((value) => {
    const api = (window as Window & { __VTK_WORKBENCH_TEST__?: WorkbenchTestApi }).__VTK_WORKBENCH_TEST__;
    if (!api) {
      throw new Error("Workbench test API is not available");
    }
    api.openDataPanel(value);
  }, request);
}

async function isShellWindowVisible(
  page: Page,
  windowId: ShellWindowIdCode
): Promise<boolean> {
  return page.evaluate((value) => {
    const api = (window as Window & { __VTK_WORKBENCH_TEST__?: WorkbenchTestApi }).__VTK_WORKBENCH_TEST__;
    if (!api) {
      throw new Error("Workbench test API is not available");
    }
    return api.isShellWindowVisible(value);
  }, windowId);
}

test("layout/reset and menu panel windows remain stable", async ({ page }) => {
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

  await page.goto("/workbench");
  await expect(page.getByTestId("workbench-root")).toHaveAttribute("data-ready", "true");

  const waitForLayoutIdle = async () => {
    await expect
      .poll(() =>
        getApiSnapshot(page).then((snapshot) => snapshot.pendingLayoutPreset)
      )
      .toBe(LAYOUT.NONE);
  };

  const waitForPanelRequestIdle = async () => {
    await expect
      .poll(() =>
        getApiSnapshot(page).then((snapshot) =>
          !snapshot.hasPendingEditorRequest &&
          !snapshot.hasPendingBuilderRequest &&
          !snapshot.hasPendingDataRequest
        )
      )
      .toBe(true);
  };

  const requestLayout = async (preset: ShellLayoutPresetCode) => {
    await requestLayoutPreset(page, preset);
    await waitForLayoutIdle();
    await waitForPanelRequestIdle();
  };

  const expectWindowVisible = async (windowId: ShellWindowIdCode) => {
    await expect
      .poll(() =>
        isShellWindowVisible(page, windowId)
      )
      .toBe(true);
  };

  const openEditorAndAssert = async (request: EditorPanelCode, windowId: ShellWindowIdCode) => {
    await openEditorPanel(page, request);
    await expectWindowVisible(windowId);
    await waitForPanelRequestIdle();
  };

  const openBuilderAndAssert = async (request: BuilderPanelCode, windowId: ShellWindowIdCode) => {
    await openBuilderPanel(page, request);
    await expectWindowVisible(windowId);
    await waitForPanelRequestIdle();
  };

  const openDataAndAssert = async (request: DataPanelCode, windowId: ShellWindowIdCode) => {
    await openDataPanel(page, request);
    await expectWindowVisible(windowId);
    await waitForPanelRequestIdle();
  };

  await requestLayout(LAYOUT.RESET);

  for (const preset of [LAYOUT.LAYOUT_1, LAYOUT.LAYOUT_2, LAYOUT.LAYOUT_3] as const) {
    await requestLayout(preset);
    await requestLayout(LAYOUT.RESET);
  }

  await openEditorAndAssert(EDITOR.ATOMS, WINDOW.CREATED_ATOMS);
  await openEditorAndAssert(EDITOR.BONDS, WINDOW.BONDS_MANAGEMENT);
  await openEditorAndAssert(EDITOR.CELL, WINDOW.CELL_INFORMATION);

  await openBuilderAndAssert(BUILDER.ADD_ATOMS, WINDOW.PERIODIC_TABLE);
  await openBuilderAndAssert(BUILDER.BRAVAIS_LATTICE_TEMPLATES, WINDOW.CRYSTAL_TEMPLATES);
  await openBuilderAndAssert(BUILDER.BRILLOUIN_ZONE, WINDOW.BRILLOUIN_ZONE_PLOT);

  await openDataAndAssert(DATA.ISOSURFACE, WINDOW.CHARGE_DENSITY_VIEWER);
  await openDataAndAssert(DATA.SURFACE, WINDOW.CHARGE_DENSITY_VIEWER);
  await openDataAndAssert(DATA.VOLUMETRIC, WINDOW.CHARGE_DENSITY_VIEWER);
  await openDataAndAssert(DATA.PLANE, WINDOW.SLICE_VIEWER);

  await expectWindowVisible(WINDOW.VIEWER);
  await expectWindowVisible(WINDOW.MODEL_TREE);

  await page.waitForTimeout(300);
  await expectWindowVisible(WINDOW.CREATED_ATOMS);
  await expectWindowVisible(WINDOW.BONDS_MANAGEMENT);
  await expectWindowVisible(WINDOW.CELL_INFORMATION);
  await expectWindowVisible(WINDOW.PERIODIC_TABLE);
  await expectWindowVisible(WINDOW.CRYSTAL_TEMPLATES);
  await expectWindowVisible(WINDOW.BRILLOUIN_ZONE_PLOT);
  await expectWindowVisible(WINDOW.CHARGE_DENSITY_VIEWER);
  await expectWindowVisible(WINDOW.SLICE_VIEWER);

  const relevantConsoleErrors = consoleErrors.filter(
    (message) => message !== "Failed to load resource: the server responded with a status of 404 (Not Found)"
  );

  expect(pageErrors).toEqual([]);
  expect(relevantConsoleErrors).toEqual([]);
});
