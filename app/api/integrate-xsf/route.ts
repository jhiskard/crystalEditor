import { NextRequest, NextResponse } from 'next/server';

export async function POST(request: NextRequest) {
    try {
        const xsfText = await request.text();
        const filename = request.headers.get('X-Filename') || 'atomic_structure.xsf';

        console.log(`[integrate-xsf] Received XSF file: ${filename}, size: ${xsfText.length} bytes`);

        // Return HTML that sends postMessage to parent window
        // This allows FlowMain.js to handle the actual Liferay integration
        const html = `
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Integrating XSF...</title>
</head>
<body>
  <p>Integrating XSF to Liferay...</p>
  <script>
    (function() {
      try {
        const xsfData = ${JSON.stringify(xsfText)};
        const filename = ${JSON.stringify(filename)};
        
        // Send to parent window (FlowMain.js)
        const payload = {
          type: 'integrate_xsf_text',
          fileName: filename,
          extension: 'xsf',
          text: xsfData
        };
        
        if (window.parent && window.parent !== window) {
          window.parent.postMessage(payload, '*');
          console.log('[integrate-xsf-bridge] Posted to parent:', filename, xsfData.length, 'bytes');
        }
        
        if (window.opener) {
          window.opener.postMessage(payload, '*');
          console.log('[integrate-xsf-bridge] Posted to opener:', filename, xsfData.length, 'bytes');
        }
        
        // Close this temporary window after a short delay
        setTimeout(function() {
          window.close();
        }, 1000);
        
      } catch (e) {
        console.error('[integrate-xsf-bridge] Error:', e);
        alert('Failed to integrate XSF: ' + e.message);
      }
    })();
  </script>
</body>
</html>
    `.trim();

        return new NextResponse(html, {
            headers: {
                'Content-Type': 'text/html; charset=utf-8',
            },
        });

    } catch (error) {
        console.error('[integrate-xsf] Error:', error);
        return NextResponse.json(
            {
                success: false,
                error: error instanceof Error ? error.message : 'Unknown error'
            },
            { status: 500 }
        );
    }
}
