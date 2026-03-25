interface VtkModuleConfig {
  print: (...args: string[]) => void;
  printErr: (...args: string[]) => void;
  canvas: HTMLCanvasElement;
  setStatus: (text: string) => void;
  onRuntimeInitialized: () => void;
  locateFile: (path: string) => string;
}

interface VtkModuleInstance {
  initIdbfs: () => void;
  saveImGuiIniFile?: () => void;
  loadImGuiIniFile?: () => void;
  FS?: {
    syncfs?: (populate: boolean, callback: (err?: unknown) => void) => void;
  };
}

declare global {
  interface Window {
    VtkModule: VtkModuleInstance;
  }
  const createViewer: (config: VtkModuleConfig) => Promise<VtkModuleInstance>;
}

export {};
