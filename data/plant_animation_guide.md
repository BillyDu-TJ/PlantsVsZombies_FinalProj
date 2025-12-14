# 植物动画配置扩展指南

## 概述

本指南说明如何为植物添加新的动画状态，实现可扩展的动画系统。

## 当前支持的动画状态

### 通用状态（所有植物）
- **idle**: 待机状态，循环播放
- **dead**: 死亡状态（可扩展）

### 射击类植物（shooter）
- **idle**: 待机
- **shoot**: 射击动画，播放一次后返回idle

### 生产类植物（producer）
- **idle**: 待机
- **produce**: 生产资源动画，播放一次后返回idle
- **grow**: 成长动画（如阳光菇）

### 即时类植物（instant）
- **idle**: 待机
- **explode**: 爆炸动画，播放一次后移除
- **activate**: 激活动画

### 防御类植物（defensive）
- **idle**: 待机
- **damaged**: 受伤状态
- **cracked**: 破裂状态（严重受伤）

## 如何添加新动画状态

### 步骤1：准备PNG序列

将新状态的GIF切片成PNG序列，例如：
```
plants/peashooter_shoot_charged/
  ├── 1.png
  ├── 2.png
  └── ...
```

### 步骤2：在plants.json中添加动画配置

在对应植物的`animations`对象中添加新状态：

```json
{
  "1001": {
    "name": "Peashooter",
    "animations": {
      "idle": { ... },
      "shoot": { ... },
      "shoot_charged": {
        "frameFormat": "plants/peashooter_shoot_charged/%d.png",
        "frameCount": 12,
        "frameDelay": 0.08,
        "loopCount": 1
      }
    }
  }
}
```

### 步骤3：在animations.json中添加完整配置

在`animations.plants`对应植物ID下添加：

```json
{
  "1001": {
    "animations": {
      "idle": { ... },
      "shoot": { ... },
      "shoot_charged": {
        "name": "shoot_charged",
        "displayName": "充能射击",
        "type": "once",
        "frameFormat": "plants/peashooter_shoot_charged/%d.png",
        "frameCount": 12,
        "startFrame": 1,
        "endFrame": 12,
        "frameDelay": 0.08,
        "loopCount": 1,
        "defaultTexture": "plants/peashooter_shoot_charged/1.png",
        "onComplete": "idle"
      }
    }
  }
}
```

## 动画状态转换示例

### 示例1：添加受伤状态

为豌豆射手添加受伤动画：

```json
{
  "1001": {
    "animations": {
      "idle": { ... },
      "shoot": { ... },
      "damaged": {
        "frameFormat": "plants/peashooter_damaged/%d.png",
        "frameCount": 8,
        "frameDelay": 0.1,
        "loopCount": 1
      }
    }
  }
}
```

### 示例2：添加多阶段攻击

为双发射手添加连续射击：

```json
{
  "1008": {
    "animations": {
      "idle": { ... },
      "shoot": { ... },
      "shoot_double": {
        "frameFormat": "plants/repeater_shoot_double/%d.png",
        "frameCount": 15,
        "frameDelay": 0.08,
        "loopCount": 1,
        "onComplete": "idle"
      }
    }
  }
}
```

### 示例3：添加状态机转换

使用`onComplete`字段实现动画链：

```json
{
  "1007": {
    "animations": {
      "idle": { ... },
      "eat": {
        "onComplete": "digest"  // 吃完后自动切换到消化
      },
      "digest": {
        "onComplete": "idle"    // 消化完后返回待机
      }
    }
  }
}
```

## 扩展建议

### 1. 血量相关动画

可以根据血量百分比切换不同状态：

```json
{
  "1004": {
    "animations": {
      "idle": { ... },
      "damaged_light": {
        "frameFormat": "plants/wallnut_damaged_light/%d.png",
        "frameCount": 16,
        "frameDelay": 0.12,
        "loopCount": -1
      },
      "damaged_medium": {
        "frameFormat": "plants/wallnut_damaged_medium/%d.png",
        "frameCount": 16,
        "frameDelay": 0.12,
        "loopCount": -1
      },
      "damaged_heavy": {
        "frameFormat": "plants/wallnut_damaged_heavy/%d.png",
        "frameCount": 16,
        "frameDelay": 0.12,
        "loopCount": -1
      }
    }
  }
}
```

### 2. 升级相关动画

为植物添加升级状态：

```json
{
  "1001": {
    "animations": {
      "idle": { ... },
      "upgrade": {
        "frameFormat": "plants/peashooter_upgrade/%d.png",
        "frameCount": 20,
        "frameDelay": 0.1,
        "loopCount": 1,
        "onComplete": "idle"
      },
      "idle_upgraded": {
        "frameFormat": "plants/peashooter_idle_upgraded/%d.png",
        "frameCount": 13,
        "frameDelay": 0.1,
        "loopCount": -1
      }
    }
  }
}
```

### 3. 特殊效果动画

添加特殊效果状态：

```json
{
  "1006": {
    "animations": {
      "idle": { ... },
      "shoot": { ... },
      "freeze_effect": {
        "frameFormat": "plants/snowpea_freeze_effect/%d.png",
        "frameCount": 10,
        "frameDelay": 0.1,
        "loopCount": 1,
        "onComplete": "idle"
      }
    }
  }
}
```

## 代码中使用新动画

### 切换动画状态

```cpp
// 伪代码示例
void Plant::playAnimation(const std::string& state) {
    auto animConfig = getAnimationConfig(this->plantId, state);
    if (animConfig) {
        // 停止当前动画
        this->stopAllActions();
        
        // 创建并播放新动画
        auto animation = createAnimationFromConfig(animConfig);
        auto animate = Animate::create(animation);
        
        // 处理动画完成后的行为
        if (animConfig->onComplete == "idle") {
            auto idleAnim = getAnimationConfig(this->plantId, "idle");
            auto idleAction = Animate::create(createAnimationFromConfig(idleAnim));
            this->runAction(Sequence::create(animate, idleAction, nullptr));
        } else if (animConfig->onComplete == "remove") {
            this->runAction(Sequence::create(
                animate,
                CallFunc::create([this]() { this->removeFromParent(); }),
                nullptr
            ));
        } else {
            // 切换到指定动画
            this->runAction(Sequence::create(
                animate,
                CallFunc::create([this, animConfig]() {
                    this->playAnimation(animConfig->onComplete);
                }),
                nullptr
            ));
        }
    }
}
```

## 注意事项

1. **命名规范**：动画状态名称使用小写字母和下划线，如`shoot_charged`、`damaged_light`
2. **帧数一致**：确保`frameCount`与实际PNG文件数量一致
3. **路径格式**：`frameFormat`必须包含`%d`占位符
4. **状态转换**：合理使用`onComplete`实现动画链
5. **性能考虑**：对于频繁切换的动画，考虑预加载到SpriteFrameCache

## 最佳实践

1. **保持一致性**：同类型植物使用相同的动画状态名称
2. **文档化**：添加新状态时更新本文档
3. **测试**：确保动画转换流畅，没有卡顿
4. **优化**：对于长动画序列，考虑使用plist和TexturePacker打包

