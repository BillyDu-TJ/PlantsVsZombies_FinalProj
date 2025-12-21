# 植物配置指南

## 如何添加新植物

### 1. 准备资源文件

#### 动画切片图片
将植物的动画切片图片放在 `Resources/plants/<植物名称>/` 目录下，命名格式为：
- `1.png`, `2.png`, `3.png`, ... (按顺序编号)

例如：
- `Resources/plants/peashooter/1.png` 到 `Resources/plants/peashooter/13.png`

#### 卡片图片
将植物的卡片图片放在 `Resources/cards/` 目录下，命名格式为：
- `card_<植物ID>.png` 或 `card_<植物名称>.png`

例如：
- `Resources/cards/card_1001.png` (Peashooter)
- `Resources/cards/card_repeaterpea.png` (Repeater)

### 2. 在 plants.json 中添加配置

在 `data/plants.json` 中添加新的植物条目，格式如下：

```json
"<植物ID>": {
  "name": "<植物名称>",
  "type": "<植物类型>",
  "hp": <生命值>,
  "cost": <阳光消耗>,
  "cooldown": <冷却时间>,
  "attack": <攻击力>,
  "attackSpeed": <攻击速度>,
  "texture": "plants/<植物名称>/1.png",
  "cardImage": "cards/card_<植物名称>.png",
  "animations": {
    "idle": {
      "frameFormat": "plants/<植物名称>/%d.png",
      "frameCount": <总帧数>,
      "frameDelay": 0.1,
      "loopCount": -1
    },
    "shoot": {
      "frameFormat": "plants/<植物名称>_shoot/%d.png",
      "frameCount": <射击动画帧数>,
      "frameDelay": 0.08,
      "loopCount": 1,
      "onComplete": "idle"
    }
  },
  "defaultAnimation": "idle"
}
```

### 3. 植物类型说明

- **shooter**: 射击类植物（如Peashooter, SnowPea）
  - 需要 `attack` 和 `attackSpeed` 字段
  - 可以有 `shoot` 动画

- **producer**: 生产类植物（如Sunflower, SunShroom）
  - 需要 `produceInterval` 字段（生产间隔）
  - 可以有 `produce` 动画

- **defensive**: 防御类植物（如WallNut, TallNut）
  - 通常没有攻击力
  - 可以有 `damaged` 和 `cracked` 动画表示受伤状态

- **instant**: 即时类植物（如CherryBomb, PotatoMine）
  - 使用后立即生效
  - 可以有 `explode` 动画

- **support**: 辅助类植物（如LilyPad）
  - 提供特殊功能，不直接攻击

### 4. 动画配置说明

#### frameFormat
动画帧的路径格式，使用 `%d` 作为帧号占位符：
- `"plants/peashooter/%d.png"` → `plants/peashooter/1.png`, `plants/peashooter/2.png`, ...

#### frameCount
动画的总帧数（从1开始计数）

#### frameDelay
每帧之间的延迟时间（秒）

#### loopCount
- `-1`: 无限循环
- `1`: 播放一次
- `>1`: 循环指定次数

#### onComplete
动画播放完成后的行为：
- `"idle"`: 切换到idle动画
- `"remove"`: 移除植物
- 不设置：动画播放完成后停止

### 5. 当前已配置的植物

| ID | 名称 | 类型 | 卡片图片 | 动画帧数 |
|----|------|------|----------|----------|
| 1001 | Peashooter | shooter | card_1001.png | 13帧idle, 8帧shoot |
| 1002 | Sunflower | producer | card_1002.png | 18帧idle |
| 1003 | CherryBomb | instant | card_cherrybomb.png | 10帧idle, 8帧explode |
| 1004 | WallNut | defensive | card_wallnut.png | 16帧idle/damaged/cracked |
| 1005 | PotatoMine | instant | card_potatomine.png | 8帧idle/ready, 10帧explode |
| 1006 | SnowPea | shooter | card_snowpea.png | 15帧idle, 8帧shoot |
| 1007 | Chomper | instant | card_bigmouthflower.png | 13帧idle, 15帧eat, 20帧digest |
| 1008 | Repeater | shooter | card_repeaterpea.png | 15帧idle, 10帧shoot |
| 1009 | PuffShroom | shooter | card_puffshroom.png | 14帧idle, 8帧shoot |
| 1010 | SunShroom | producer | card_sunshroom.png | 10帧idle, 12帧produce, 15帧grow |
| 1011 | FumeShroom | shooter | card_scaredyshroom.png | 17帧idle, 10帧shoot |
| 1012 | Spikeweed | defensive | card_spikeweed.png | 19帧idle, 12帧attack |
| 1013 | TallNut | defensive | card_threepeashooter.png | 14帧idle/damaged/cracked |
| 1014 | LilyPad | support | card_1001.png | 3帧idle |

### 6. 在游戏中添加植物到卡片槽

在 `Classes/Scenes/GameScene.cpp` 的 `init()` 方法中，找到以下代码：

```cpp
std::vector<int> plantIds = { 1001, 1002, 1008 }; // 添加植物ID
```

将新的植物ID添加到这个列表中，例如：

```cpp
std::vector<int> plantIds = { 1001, 1002, 1008, 1004, 1006 }; // Peashooter, Sunflower, Repeater, WallNut, SnowPea
```

### 7. 特殊植物行为

#### Repeater (1008)
Repeater会一次发射两个豌豆。这个逻辑在 `GameScene::createPlant()` 中实现：

```cpp
if (currentPlantId == 1008) {
    // 发射第一个豌豆
    this->createBullet(pos, damage);
    // 稍微延迟发射第二个豌豆
    this->runAction(Sequence::create(
        DelayTime::create(0.05f),
        CallFunc::create([this, pos, damage]() {
            this->createBullet(pos, damage);
        }),
        nullptr
    ));
}
```

如果需要为其他植物添加特殊行为，可以在相同的位置添加相应的逻辑。

### 8. 注意事项

1. **文件路径大小写敏感**：确保JSON中的路径与实际文件系统中的路径大小写一致
2. **帧数统计**：确保 `frameCount` 与实际文件数量一致（从1开始计数）
3. **卡片图片**：如果卡片图片不存在，系统会使用默认的 `card_generic.png` 或根据植物ID自动查找 `card_<ID>.png`
4. **动画路径**：确保动画切片图片的路径格式正确，使用 `%d` 作为帧号占位符

