import { NextResponse } from 'next/server';

export async function GET() {
  try {
    // Basic health check - can be extended with database connectivity, etc.
    return NextResponse.json({
      status: 'ok',
      timestamp: new Date().toISOString(),
      uptime: process.uptime()
    });
  } catch {
    return NextResponse.json(
      { status: 'error', message: 'Health check failed' },
      { status: 500 }
    );
  }
}