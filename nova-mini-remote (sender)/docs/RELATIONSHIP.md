# Relationship between Nova Mini and Nova Mini Remote

## Overview
This document describes how Nova Mini and Nova Mini Remote interact, focusing on the updated game mode and control flow.

## Device Roles

### Nova Mini (Receiver)
- Serves as the hardware controller.
- Monitors game inputs and executes game logic.
- Provides LED and buzzer feedback based on game state changes.

### Nova Mini Remote (Sender)
- Offers a user interface to initiate and control the game.
- Sends local game commands to Nova Mini.
- Updates the game state through real-time interactions.

## Game Mode Enhancements
- The Nova Mini Remote now features an enhanced game mode with improved state management.
- The legacy resetGame() function has been removed.
- A dedicated reset stage (SIMONA_STAGE_RESET) replaces former reset logic and is activated via remote button presses.
- SimonaMessage is used to communicate game stages, levels, and user inputs in real time, ensuring clear feedback and responsive control.

## Workflow
1. Nova Mini Remote issues a game command (e.g., start a new round or reset the game).
2. Nova Mini receives the command, processes it, and updates its game state.
3. SimonaMessage coordinates feedback by triggering appropriate LED, buzzer, and button responses.
4. Together, these interactions create an engaging, interactive game experience.
