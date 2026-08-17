#!/usr/bin/env node
/**
 * build-docs.js — 用 npm 包 doxygen 生成 wbwlib 文档（docs/doxygen/）。
 * 主题：doxygen-awesome-css；数学公式：MathJax；图表：Graphviz @dot。
 * 用法：node tools/build-docs.js   （或 npm run docs）
 */
'use strict';

const { spawnSync } = require('child_process');
const path = require('path');
const fs = require('fs');

const ROOT = path.resolve(__dirname, '..');
const EXE = path.join(ROOT, 'node_modules', 'doxygen', 'dist', '1.9.1', 'doxygen.exe');
const CFG = path.join(ROOT, 'Doxyfile');

if (!fs.existsSync(EXE)) {
  console.error('doxygen 未安装：请先执行 npm install（node_modules/doxygen/dist/1.9.1/doxygen.exe）');
  process.exit(1);
}

const r = spawnSync(EXE, [CFG], { cwd: ROOT, stdio: 'inherit' });
if (r.status !== 0) process.exit(r.status || 1);
console.log('文档已生成：docs/doxygen/index.html（doxygen-awesome-css + MathJax + Graphviz）');