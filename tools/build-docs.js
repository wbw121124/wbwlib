#!/usr/bin/env node
/**
 * build-docs.js — 生成 wbwlib 文档（docs/doxygen/）。
 * 主题：doxygen-awesome-css；公式：本地 MathJax；图表：Graphviz @dot。
 * 用法：node tools/build-docs.js   （或 npm run docs）
 *
 * 跨平台：Windows 用 npm 包自带二进制 node_modules/doxygen/dist/1.9.1/doxygen.exe；
 *         Linux/macOS 用系统 doxygen（CI 里 sudo apt-get install -y doxygen graphviz）。
 */
'use strict';

const { spawnSync } = require('child_process');
const path = require('path');
const fs = require('fs');

const ROOT = path.resolve(__dirname, '..');
const CFG = path.join(ROOT, 'Doxyfile');
const isWin = process.platform === 'win32';

let exe = 'doxygen';
if (isWin) {
  exe = path.join(ROOT, 'node_modules', 'doxygen', 'dist', '1.9.1', 'doxygen.exe');
  if (!fs.existsSync(exe)) {
    console.error('doxygen 未安装：请先执行 npm install（需要 node_modules/doxygen/dist/1.9.1/doxygen.exe）');
    process.exit(1);
  }
} else {
  // 用系统 doxygen（apt/brew），避免命中 node_modules/.bin 里的 npm 包装器
  const r = spawnSync('which', ['-a', 'doxygen'], { encoding: 'utf8' });
  const candidates = r.status === 0
    ? r.stdout.trim().split('\n').filter(p => p && !p.includes('node_modules'))
    : [];
  if (candidates.length === 0) {
    console.error('未找到系统 doxygen：请先安装（Ubuntu: sudo apt-get install -y doxygen）');
    process.exit(1);
  }
  exe = candidates[0];
}

// Linux 上用 PATH 里的 dot，无需 Doxyfile 里 Windows 专用 DOT_PATH
let cfg = fs.readFileSync(CFG, 'utf8');
if (!isWin) {
  cfg = cfg.replace(/^DOT_PATH\s*=.*$/m, 'DOT_PATH =');
}
const cfgPath = path.join(ROOT, '.doxyfile-tmp');
fs.writeFileSync(cfgPath, cfg);

const r = spawnSync(exe, [cfgPath], { cwd: ROOT, stdio: 'inherit' });
fs.unlinkSync(cfgPath);
if (r.status !== 0) process.exit(r.status || 1);
console.log('文档已生成：docs/doxygen/index.html（doxygen-awesome-css + 本地 MathJax + Graphviz）');