# 🥚 Catch The Eggs

> A 2D arcade game built with OpenGL and C++ for CSE 426 - Computer Graphics Lab 

---

## 📸 Overview

A chicken sits on a bamboo stick and lays eggs. Your job is to catch as many eggs as possible using a basket before the timer runs out. Watch out for poop — catching it costs you a life and deducts points! Wind blows objects sideways and the game gets faster over time.

---

## 🎮 How to Play

Use your basket to catch falling eggs and avoid poop. You have **60 seconds** and **3 lives**. Lose all lives or run out of time and the game ends.

### Controls

| Input | Action |
|-------|--------|
| Mouse movement | Move basket left and right |
| Left / Right arrow keys | Move basket left and right |
| A / D keys | Move basket left and right |
| Space | Pause and resume the game |
| R | Restart from game over screen |
| Esc | Return to menu from any screen |
| Enter | Start game from menu |

---

## 🥚 Object Types

| Object | Appearance | Effect |
|--------|-----------|--------|
| Normal Egg | White circle | +1 point |
| Blue Egg | Light blue circle | +5 points |
| Golden Egg | Gold circle with glow ring | +10 points |
| Poop 💩 | Brown stacked circles with smell lines | -10 points, -1 life |

---

## 🖥️ Screens

- **Main Menu** — Start, Resume, Help and Exit buttons with animated background
- **Help Screen** — Lists all controls and object types
- **Countdown Screen** — 3-2-1-GO! animation before each game starts
- **Game Screen** — Main gameplay with HUD showing score, time, lives and wind
- **Pause Screen** — Semi-transparent overlay with resume instructions
- **Game Over Screen** — Shows final score, high score and restart option

---

## ✨ Features

- Animated chicken walking back and forth on bamboo stick
- Scrolling clouds, distant hills and farmhouse background
- Three egg types with different point values and visual styles
- Poop object with stacked brown circles and green smell lines
- Heart icon lives display in HUD (red = alive, gray = lost)
- Wind system that randomly changes direction and strength every few seconds
- Wind arrow indicator in HUD turns red when wind is strong
- Particle burst effects on every catch
- Floating score text that drifts upward on catch
- 3-2-1-GO countdown with pulsing animation before game starts
- Screen shake effect when poop is caught
- Background music using procedurally generated C-pentatonic melody
- Unique sound effect for each object type on catch
- High score persists across multiple games in the same session
- Spawn rate increases gradually over time making the game harder

---

## 🔧 Build Instructions

### Requirements
- Windows OS
- Code::Blocks IDE with MinGW-w64 compiler
- OpenGL / GLUT libraries (freeglut)

### Compiler Settings in Code::Blocks

**Project → Build Options → Compiler Settings → Other options:**
```
-std=c++11
```

**Project → Build Options → Linker Settings → Other linker options:**
```
-lglut32
-lglu32
-lopengl32
-lwinmm
```

### Command Line Build
```bash
g++ -std=c++11 main.cpp -o CatchTheEggs -lglut32 -lglu32 -lopengl32 -lwinmm
```

---

## 📁 Project Structure

```
catch-the-eggs/
├── main.cpp          # Complete game source code
└── README.md         # Project documentation
```

---

## 🏫 Course Information

- **Course:** CSE 426 - Computer Graphics Lab
- **University:** University of Asia Pacific
