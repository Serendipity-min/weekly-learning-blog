# 📚 每周学习博客 · Weekly Learning Blog

> 嵌入式（STM32）与硬件设计学习记录：**每周一篇周记总结 + 问题与知识沉淀**。
> 用 git 版本控制管理，每周提交一次，见证从电路基础到完整项目的成长轨迹。

## 🗓️ 文章索引

| 周次 | 日期 | 主题 | 周记 | 问题沉淀 |
|---|---|---|---|---|
| [2026-W35](weekly/2026-W35/) | 08-24 ~ 08-30 | 电路基础 · 模拟/数字电路 · PCB 制作 · AI×嘉立创EDA 工具链 | [周记总结](weekly/2026-W35/index.md) | [AI 智能体 + MCP + 技能驱动嘉立创 EDA](weekly/2026-W35/issues/ai-agent-mcp-jlceda.md) |

## 📁 仓库结构

```
weekly-learning-blog/
├── README.md               # 本页：博客首页与索引
├── LICENSE
├── .gitignore
└── weekly/                 # 每周文章（按 ISO 周目录）
    └── 2026-W35/           # 例：第 35 周
        ├── index.md        # 本周学习总结（周记）
        └── issues/         # 本周问题与知识沉淀
            └── *.md
```

## 🎯 关于本博客

- **定位**：个人学习记录与分享，重点覆盖：
  - STM32 嵌入式开发（F407 开发板，当前处于电路基础阶段）
  - 电路基础 / 模拟电路 / 数字电路 / PCB 设计（嘉立创 EDA）
  - AI 编程智能体（Antigravity / Codex / DeepSeek Harness）× MCP / 技能 × 硬件设计自动化
- **更新频率**：每周一篇周记 + 视情况补充问题沉淀文章。

## 🔧 维护指南（每周例行）

1. 新建目录 `weekly/<ISO周>/`（如 `2026-W36`）；
2. 编写 `index.md`（周记总结）与 `issues/` 下的问题文档；
3. 更新本 README 的索引表格；
4. 提交并推送：
   ```bash
   git add -A
   git commit -m "docs: 2026-W36 周记与问题沉淀"
   git push
   ```

## 📜 许可

[MIT](LICENSE) © 2026 Serendipity-min
