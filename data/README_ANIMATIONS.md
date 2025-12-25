# 动画资源配置说明

## 文件结构

本项目使用JSON文件来配置动画资源，主要文件包括：

1. **animations.json** - 完整的动画资源配置文件
2. **animation_schema.json** - JSON Schema架构定义（用于验证）
3. **zombies.json** - 僵尸数据（已包含动画配置）
4. **plants.json** - 植物数据（可扩展添加动画配置）

## 动画配置格式

### 基本结构

```json
{
  "animations": {
    "zombies": {
      "2001": {
        "id": "2001",
        "name": "NormalZombie",
        "animations": {
          "walk": {
            "frameFormat": "zombies/normalzombie/normalzombie_walk/%d.png",
            "frameCount": 22,
            "frameDelay": 0.1,
            "loopCount": -1
          }
        },
        "defaultAnimation": "walk"
      }
    }
  }
}
```

### 字段说明

#### 动画配置字段

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `name` | string | 是 | 动画名称（内部标识），如 "walk", "eat", "dead" |
| `displayName` | string | 否 | 动画显示名称，用于UI显示 |
| `type` | string | 是 | 动画类型：`loop`(循环)、`once`(播放一次)、`pingpong`(往返) |
| `frameFormat` | string | 是 | 帧文件路径格式，必须包含`%d`占位符，如 `"zombies/normalzombie/normalzombie_walk/%d.png"` |
| `frameCount` | integer | 是 | 总帧数 |
| `startFrame` | integer | 否 | 起始帧序号（默认1） |
| `endFrame` | integer | 否 | 结束帧序号（默认等于frameCount） |
| `frameDelay` | number | 是 | 每帧之间的延迟时间（秒），如 0.1 表示每帧0.1秒 |
| `loopCount` | integer | 是 | 循环次数：`-1`表示无限循环，`1`表示播放一次 |
| `defaultTexture` | string | 否 | 默认显示的静态图片路径（通常是第一帧） |
| `onComplete` | string | 否 | 动画完成后的行为：`remove`(移除)、`idle`(切换到待机)、`none`(无操作) |

## 当前配置示例

### 僵尸动画（zombies/normalzombie/）

已配置的僵尸动画包括：

1. **walk（行走）**
   - 路径：`zombies/normalzombie/normalzombie_walk/%d.png`
   - 帧数：22帧（1.png 到 22.png）
   - 延迟：0.1秒/帧
   - 循环：无限循环

2. **eat（吃植物）**
   - 路径：`zombies/normalzombie/normalzombie_eat/%d.png`
   - 帧数：21帧
   - 延迟：0.1秒/帧
   - 循环：无限循环

3. **dead（死亡）**
   - 路径：`zombies/normalzombie/normalzombie_dead/%d.png`
   - 帧数：20帧
   - 延迟：0.12秒/帧
   - 循环：播放一次

4. **stand（站立）**
   - 路径：`zombies/normalzombie/normalzombie_stand/%d.png`
   - 帧数：11帧
   - 延迟：0.15秒/帧
   - 循环：无限循环

## 在代码中使用

### 伪代码示例

```cpp
// 1. 加载动画配置
AnimationConfig config = loadAnimationConfig("data/animations.json");

// 2. 获取僵尸的行走动画配置
AnimationData walkAnim = config.getAnimation("zombies", "2001", "walk");

// 3. 创建精灵
Sprite* zombie = Sprite::create(walkAnim.defaultTexture);

// 4. 创建动画
Animation* animation = createAnimationFromConfig(walkAnim);
// animation包含：
// - 帧序列：zombies/normalzombie/normalzombie_walk/1.png 到 zombies/normalzombie/normalzombie_walk/22.png
// - 每帧延迟：0.1秒
// - 循环：无限循环

// 5. 运行动画
Animate* animate = Animate::create(animation);
zombie->runAction(animate);
```

### 动画切换示例

```cpp
// 切换到吃动画
AnimationData eatAnim = config.getAnimation("zombies", "2001", "eat");
zombie->stopAllActions();
Animation* eatAnimation = createAnimationFromConfig(eatAnim);
zombie->runAction(Animate::create(eatAnimation));

// 播放死亡动画（只播放一次）
AnimationData deadAnim = config.getAnimation("zombies", "2001", "dead");
zombie->stopAllActions();
Animation* deadAnimation = createAnimationFromConfig(deadAnim);
zombie->runAction(Sequence::create(
    Animate::create(deadAnimation),
    CallFunc::create([zombie]() {
        zombie->removeFromParent(); // 死亡后移除
    }),
    nullptr
));
```

## 添加新动画

### 步骤1：准备PNG序列

将GIF切片成PNG序列，例如：
- `plants/peashooter_frames/1.png`
- `plants/peashooter_frames/2.png`
- ...
- `plants/peashooter_frames/15.png`

### 步骤2：在JSON中添加配置

在 `animations.json` 或对应的实体JSON文件中添加：

```json
{
  "1001": {
    "name": "Peashooter",
    "animations": {
      "idle": {
        "frameFormat": "plants/peashooter_frames/%d.png",
        "frameCount": 15,
        "frameDelay": 0.1,
        "loopCount": -1
      }
    },
    "defaultAnimation": "idle"
  }
}
```

### 步骤3：验证配置

确保：
- `frameFormat` 中的 `%d` 会被正确替换为帧序号
- `frameCount` 与实际PNG文件数量一致
- 文件路径相对于Resources目录正确

## 注意事项

1. **文件命名**：PNG文件必须按顺序命名（1.png, 2.png, 3.png...）
2. **路径格式**：`frameFormat` 必须包含 `%d` 占位符
3. **帧数统计**：确保 `frameCount` 与实际文件数量一致
4. **性能优化**：对于频繁使用的动画，建议预加载到SpriteFrameCache
5. **循环设置**：
   - 待机、行走等持续动作：`loopCount: -1`
   - 攻击、死亡等一次性动作：`loopCount: 1`

## 扩展建议

未来可以扩展的功能：

1. **动画事件**：在特定帧触发事件（如攻击判定帧）
2. **动画混合**：支持动画之间的平滑过渡
3. **帧范围**：支持只播放部分帧（如帧1-10）
4. **速度曲线**：支持动画加速/减速效果
5. **音效绑定**：在动画配置中绑定音效

