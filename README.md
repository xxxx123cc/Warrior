  视频演示地址：   https://b23.tv/mNK7YjF
  - 基于 Unreal Engine 5.5 开发，使用 C++ 编写核心系统（约70+源文件），蓝图负责资产配置与表现层逻辑
  - 深度集成 Gameplay Ability System (GAS) 框架：自定义
  AbilitySystemComponent、AttributeSet（生命/怒气/攻防属性）、ExecutionCalculation
  伤害计算（含连击加成），实现属性驱动的技能体系
  - 实现完整战斗系统：轻重攻击连招（连击数递增伤害）、格挡系统（基于朝向点积的方向判定，含不可格挡攻击处理）、目标锁定（
  Box Trace检测 + 最近优先选择 + 左右切换）、武器碰撞检测与碰撞冲突处理
  - 设计 波次生存游戏模式：基于 DataTable 配置的敌人波次表，异步加载敌人资源，利用 NavigationSystem
  随机可达点生成，敌人死亡后自动补充
  - 构建 AI 系统：使用 行为树 + 感知组件（视觉感知 360°/5000 范围），配合 Detour Crowd Avoidance
  群体避让，实现敌人追踪、攻击、面向目标等行为
  - 采用 Enhanced Input + GameplayTag 统一输入绑定，技能输入通过 GameplayTag 路由到 ASC 激活，支持可切换的输入映射上下文
  - 使用 Niagara 粒子系统实现弹道与命中特效，Motion Warping 驱动动画根骨骼位移，GameplayCue 管理表现层事件同步
  - 实现武器切换系统：武器数据资产关联动画层、输入映射、技能集及可缩放伤害，切换时自动授权/回收技能

