"use client";
import Script from "next/script";
import { useEffect, useRef, useState } from "react";

type VtkWasmModule = {
  FS: {
    createDataFile: (
      parent: string,
      name: string,
      data: Uint8Array,
      canRead: boolean,
      canWrite: boolean,
      canOwn: boolean
    ) => void;
    unlink?: (path: string) => void;
    analyzePath?: (path: string) => { exists?: boolean };
    syncfs?: (populate: boolean, callback: (err?: unknown) => void) => void;
  };
  loadArrayBuffer: (name: string, deleteFile?: boolean) => void;
  loadChgcarFile?: (name: string) => void;
  initIdbfs: () => void;
  saveImGuiIniFile?: () => void;
  loadImGuiIniFile?: () => void;
  exportXSF?: (fileName: string) => void;
  integrateXSF?: (fileName: string) => void;
  getStructureCount?: () => number;
  getCurrentStructureId?: () => number;
  isStructureVisible?: (id: number) => boolean;
  setStructureVisible?: (id: number, visible: boolean) => void;
  getMeshCount?: () => number;
  hasChargeDensity?: () => boolean;
  resize?: (width: number, height: number) => void;
};
// 전역 보강 대신 안전한 캐스팅을 위한 로컬 인터페이스
interface WindowWithVtk {
  VtkModule?: unknown;
}

type PendingImportKind = "xsf" | "chgcar";

type PendingImport = {
  text: string;
  fileName: string;
  kind: PendingImportKind;
};

type WorkbenchTestApi = {
  importXsfText: (fileName: string, text: string) => void;
  importChgcarText: (fileName: string, text: string) => void;
  getStructureCount: () => number;
  getCurrentStructureId: () => number;
  isStructureVisible: (id: number) => boolean;
  setStructureVisible: (id: number, visible: boolean) => void;
  getMeshCount: () => number;
  hasChargeDensity: () => boolean;
};

interface WindowWithVtk {
  __VTK_WORKBENCH_TEST__?: WorkbenchTestApi;
}

export default function Workbench() {
  const vtkCanvasRef = useRef<HTMLCanvasElement>(null);
  const pendingImportRef = useRef<PendingImport | null>(null);
  const vtkModuleRef = useRef<VtkWasmModule | null>(null);
  const idbfsMountedRef = useRef(false);
  const [ready, setReady] = useState(false);

  const removeExistingFile = (module: VtkWasmModule, fileName: string) => {
    const filePath = `/${fileName}`;
    const pathState = module.FS.analyzePath?.(filePath);
    if (pathState?.exists) {
      module.FS.unlink?.(filePath);
    }
  };

  const importTextIntoModule = (
    module: VtkWasmModule,
    fileName: string,
    text: string,
    kind: PendingImportKind
  ) => {
    const bytes = new TextEncoder().encode(text);
    removeExistingFile(module, fileName);
    module.FS.createDataFile("/", fileName, bytes, true, false, true);

    if (kind === "chgcar") {
      if (!module.loadChgcarFile) {
        throw new Error("loadChgcarFile binding is not available");
      }
      module.loadChgcarFile(fileName);
      return;
    }

    module.loadArrayBuffer(fileName, true);
  };

  const handleContextMenu = (e: React.FormEvent<HTMLCanvasElement>) => {
    e.preventDefault(); // To prevent the browser context menu
  };

  const handleResize = () => {
    if (vtkCanvasRef.current) {
      // iframe 내부에서는 window.innerWidth가 정확하지 않을 수 있으므로
      // document.documentElement.clientWidth 등을 함께 고려하거나
      // 단순히 100%로 설정하고 부모 컨테이너에 맞깁니다.

      const width = window.innerWidth;
      const height = window.innerHeight;

      // Set canvas display size
      vtkCanvasRef.current.style.width = "100%";
      vtkCanvasRef.current.style.height = "100%";

      // Set canvas buffer size (considering device pixel ratio)
      const pixelRatio = window.devicePixelRatio || 1;
      vtkCanvasRef.current.width = width * pixelRatio;
      vtkCanvasRef.current.height = height * pixelRatio;

      console.log(`Canvas resized: ${width}x${height} (display), ${vtkCanvasRef.current.width}x${vtkCanvasRef.current.height} (buffer)`);

      // Notify WASM module if it's ready and has a resize function
      const vm = (window as unknown as WindowWithVtk).VtkModule as VtkWasmModule | undefined;
      if (vm && typeof vm.resize === "function") {
        vm.resize(vtkCanvasRef.current.width, vtkCanvasRef.current.height);
      }
    }
  };



  // Workbench
  useEffect(() => {
    handleResize();
    window.addEventListener("resize", handleResize);
    document.addEventListener("fullscreenchange", handleResize);

    const persistImGuiLayout = (reason: string) => {
      const module = vtkModuleRef.current;
      if (!module || !idbfsMountedRef.current) {
        return;
      }

      try {
        module.saveImGuiIniFile?.();
      } catch (err) {
        console.error(`Failed to save ImGui ini (${reason})`, err);
        return;
      }

      if (typeof module.FS.syncfs === "function") {
        module.FS.syncfs(false, (err?: unknown) => {
          if (err) {
            console.error(`Failed to sync ImGui ini to IDBFS (${reason})`, err);
          }
        });
      }
    };

    const handlePageHide = () => {
      persistImGuiLayout("pagehide");
    };

    const handleBeforeUnload = () => {
      persistImGuiLayout("beforeunload");
    };

    const handleVisibilityChange = () => {
      if (document.visibilityState === "hidden") {
        persistImGuiLayout("visibilitychange");
      }
    };

    window.addEventListener("pagehide", handlePageHide);
    window.addEventListener("beforeunload", handleBeforeUnload);
    document.addEventListener("visibilitychange", handleVisibilityChange);

    // 메시지 핸들러: 부모/오프너로부터 전달되는 XSF를 수신하여 즉시 임포트
    const handleMessage = async (e: MessageEvent<unknown>) => {
      const data = e.data as unknown;
      // 허용 오리진은 서버 설정에 따라 필터할 수 있으나, 팝업 통합을 위해 우선 수신
      // if (e.origin !== window.location.origin) return;

      try {
        const typeLabel =
          typeof data === "object" && data !== null && "type" in data
            ? String((data as Record<string, unknown>).type)
            : typeof data;
        console.debug('[WorkbenchBridge][child] message received from', (e as MessageEvent).origin, 'type=', typeLabel);
        let xsfText: string | null = null;
        let fileName = "atomic_structure.xsf";

        if (typeof data === "string") {
          // 문자열 URL 전달된 경우
          console.debug('[WorkbenchBridge][child] fetching XSF from url');
          xsfText = await fetch(data, { cache: "no-cache" }).then((r) => r.text());
        } else if (data && typeof data === "object") {
          const obj = data as Record<string, unknown>;
          if (typeof obj.url === "string") {
            if (typeof obj.fileName === "string" && obj.fileName) {
              fileName = (obj.fileName as string).endsWith(".xsf") ? (obj.fileName as string) : `${obj.fileName as string}.xsf`;
            }
            console.debug('[WorkbenchBridge][child] fetching XSF from url object, fileName=', fileName);
            xsfText = await fetch(obj.url as string, { cache: "no-cache" }).then((r) => r.text());
          } else if (obj.type === "integrate_xsf_text" && typeof obj.text === "string") {
            if (typeof obj.fileName === "string" && obj.fileName) {
              fileName = (obj.fileName as string).endsWith(".xsf") ? (obj.fileName as string) : `${obj.fileName as string}.xsf`;
            }
            xsfText = obj.text as string;
            console.debug('[WorkbenchBridge][child] received integrate_xsf_text, fileName=', fileName, 'length=', xsfText.length);
          }
        }

        if (xsfText && xsfText.trim().length > 0) {
          const doImport = () => {
            try {
              // VtkModule이 준비되었다면 MemFS에 파일을 만들고 로드
              const vm = (window as unknown as WindowWithVtk).VtkModule as VtkWasmModule;
              if (!vm?.FS || !vm?.loadArrayBuffer) {
                throw new Error("VtkModule is not ready");
              }
              importTextIntoModule(vm, fileName, xsfText as string, "xsf");
              console.debug('[WorkbenchBridge][child] XSF imported via test seam', fileName);
            } catch (err) {
              console.error("Failed to import XSF into VtkModule", err);
            }
          };

          // VtkModule 준비 상태에 따라 즉시 임포트 또는 보관
          if ((window as unknown as WindowWithVtk).VtkModule) {
            doImport();
          } else {
            pendingImportRef.current = { text: xsfText, fileName, kind: "xsf" };
            console.debug('[WorkbenchBridge][child] VtkModule not ready, pending XSF stored', fileName, 'length=', xsfText.length);
          }
        }
      } catch (err) {
        console.error("Workbench message handler error", err);
      }
    };
    window.addEventListener("message", handleMessage);

    // 부모/오프너에게 XSF 데이터를 요청
    try {
      const req = { type: "request_xsf" };
      window.parent?.postMessage(req, "*");
      window.opener?.postMessage(req, "*");
      console.debug('[WorkbenchBridge][child] request_xsf sent (on mount)');
    } catch (err) {
      console.error("Failed to request XSF from parent/opener", err);
    }

    return () => {
      window.removeEventListener("resize", handleResize);
      document.removeEventListener("fullscreenchange", handleResize);
      window.removeEventListener("message", handleMessage);
      window.removeEventListener("pagehide", handlePageHide);
      window.removeEventListener("beforeunload", handleBeforeUnload);
      document.removeEventListener("visibilitychange", handleVisibilityChange);
      delete (window as unknown as WindowWithVtk).__VTK_WORKBENCH_TEST__;
    };
  }, []);

  const handleLoad = () => {
    console.log("Success to find VTK wasm");

    const moduleConfig = {
      print: (function () {
        return function (...args: string[]) {
          const text = args.join(" ");
          console.log(text);
        };
      })(),
      printErr: function (...args: string[]) {
        const text = args.join(" ");
        console.error(text);
      },
      canvas: (function () {
        const canvas = document.getElementById("canvas") as HTMLCanvasElement;

        // Set initial canvas size before creating WebGL context
        if (canvas.width === 0 || canvas.height === 0) {
          canvas.width = Math.max(window.innerWidth, 800);
          canvas.height = Math.max(window.innerHeight - 56, 600);
        }

        canvas.addEventListener(
          "webglcontextlost",
          function (e) {
            alert("WebGL context lost, please reload the page");
            e.preventDefault();
          },
          false
        );

        // Try to create WebGL2 context first, fallback to WebGL1
        let gl: WebGLRenderingContext | WebGL2RenderingContext | null = canvas.getContext("webgl2", {
          alpha: false,
          depth: true,
          antialias: false,
          preserveDrawingBuffer: true,
        });

        let webglVersion = "WebGL2";

        if (!gl) {
          console.warn("WebGL2 not supported, trying WebGL1...");
          gl = canvas.getContext("webgl", {
            alpha: false,
            depth: true,
            antialias: false,
            preserveDrawingBuffer: true,
          });
          webglVersion = "WebGL1";
        }

        if (!gl) {
          console.error("Neither WebGL2 nor WebGL1 is supported");
          alert("WebGL is not supported. Please use a browser that supports WebGL.");
          // Return a dummy canvas to satisfy TypeScript, VTK will handle the error
          const dummyCanvas = document.createElement('canvas');
          dummyCanvas.width = 800;
          dummyCanvas.height = 600;
          return dummyCanvas;
        }

        console.log(`${webglVersion} context created successfully`);
        console.log("WebGL Version:", gl.getParameter(gl.VERSION));
        console.log("WebGL Renderer:", gl.getParameter(gl.RENDERER));

        return canvas;
      })(),
      setStatus: function (text: string) {
        if (text.length) {
          console.log("Status:", text);
        }
      },
      onRuntimeInitialized: function () {
        console.log("VtkModule runtime initialized.");

        const canvas = document.getElementById("canvas") as HTMLCanvasElement;

        // focus on the canvas to grab keyboard inputs.
        canvas.setAttribute("tabindex", "0");

        // grab focus when the render window region receives mouse clicks.
        canvas.addEventListener("click", () => canvas.focus());
      },
      locateFile: function (path: string) {
        if (path.endsWith(".wasm")) {
          return `/wasm/${path}?v=${new Date().getTime()}`;
        }
        return `/wasm/${path}`;
      },
    };

    createViewer(moduleConfig).then((module) => {
      console.log("VtkModule instance initialized.");
      const vtkModule = module as unknown as VtkWasmModule;

      // Export this module to global module (store as unknown)
      (window as unknown as WindowWithVtk).VtkModule = module as unknown;
      vtkModuleRef.current = vtkModule;

      // Mount IDBFS
      vtkModule.initIdbfs();
      idbfsMountedRef.current = true;

      const restoreImGuiLayout = () => {
        try {
          vtkModule.loadImGuiIniFile?.();
        } catch (err) {
          console.error("Failed to load ImGui ini", err);
        }
      };

      if (typeof vtkModule.FS.syncfs === "function") {
        vtkModule.FS.syncfs(true, (err?: unknown) => {
          if (err) {
            console.error("Failed to sync ImGui ini from IDBFS", err);
          }
          restoreImGuiLayout();
        });
      } else {
        restoreImGuiLayout();
      }

      handleResize();

      // Module ready
      setReady(true);

      (window as unknown as WindowWithVtk).__VTK_WORKBENCH_TEST__ = {
        importXsfText: (fileName: string, text: string) =>
          importTextIntoModule(vtkModule, fileName, text, "xsf"),
        importChgcarText: (fileName: string, text: string) =>
          importTextIntoModule(vtkModule, fileName, text, "chgcar"),
        getStructureCount: () => vtkModule.getStructureCount?.() ?? 0,
        getCurrentStructureId: () => vtkModule.getCurrentStructureId?.() ?? -1,
        isStructureVisible: (id: number) => vtkModule.isStructureVisible?.(id) ?? false,
        setStructureVisible: (id: number, visible: boolean) =>
          vtkModule.setStructureVisible?.(id, visible),
        getMeshCount: () => vtkModule.getMeshCount?.() ?? 0,
        hasChargeDensity: () => vtkModule.hasChargeDensity?.() ?? false,
      };

      // 모듈 초기화 이후, 대기 중이던 XSF가 있으면 즉시 임포트
      if (pendingImportRef.current) {
        try {
          importTextIntoModule(
            vtkModule,
            pendingImportRef.current.fileName,
            pendingImportRef.current.text,
            pendingImportRef.current.kind
          );
          console.debug('[WorkbenchBridge][child] pending import applied after init', pendingImportRef.current.fileName);
        } catch (e) {
          console.error("Failed to import pending file", e);
        } finally {
          pendingImportRef.current = null;
        }
      }

      // 혹시 부모가 아직 응답하지 않았다면, 모듈 초기화 후 한 번 더 요청
      try {
        const req = { type: "request_xsf" };
        window.parent?.postMessage(req, "*");
        window.opener?.postMessage(req, "*");
        console.debug('[WorkbenchBridge][child] request_xsf sent (after init)');
      } catch (err) {
        console.error("Failed to request XSF after init", err);
      }
    });

    //! Check browser support.
    function isWasmSupported() {
      try {
        if (
          typeof WebAssembly === "object" &&
          typeof WebAssembly.instantiate === "function"
        ) {
          const aDummyModule = new WebAssembly.Module(
            Uint8Array.of(0x0, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00)
          );
          if (aDummyModule instanceof WebAssembly.Module) {
            return (
              new WebAssembly.Instance(aDummyModule) instanceof
              WebAssembly.Instance
            );
          }
        }
      } catch (e) {
        console.error(e);
      }
      return false;
    }

    if (!isWasmSupported()) {
      alert(
        "Your browser cannot support WebAssembly! Please install a modern browser."
      );
    }
  };

  return (
    <div
      className="h-full"
      data-testid="workbench-root"
      data-ready={ready ? "true" : "false"}
    >
      <div></div>
      <canvas
        ref={vtkCanvasRef}
        onContextMenu={handleContextMenu}
        id="canvas"
        className="outline-none block"
        style={{
          width: "100%",
          height: "100%",
          minWidth: "400px",
          minHeight: "300px"
        }}
      ></canvas>
      <Script
        src={`/wasm/VTK-Workbench.js?v=${new Date().getTime()}`}
        strategy="afterInteractive"
        onLoad={handleLoad}
        onError={(e) => {
          console.error("Failed to find VTK wasm", e);
        }}
      />
    </div>
  );
}
