#pragma once

enum class InputEventSource { NONE, KEYBOARD };

class InputEvent {
public:
  explicit InputEvent(const InputEventSource source) : mSource{source} {}

  [[nodiscard]] InputEventSource getSource() const { return mSource; }

private:
  InputEventSource mSource;
};

enum class KeyCode { NONE, ESCAPE, UP, DOWN, LEFT, RIGHT, H, J, K, L, W, S, A, D };
enum class KeyAction { NONE, DOWN, UP };

class KeyInputEvent : public InputEvent {
public:
  KeyInputEvent(KeyCode code, KeyAction action)
      : InputEvent{InputEventSource::KEYBOARD}, mCode{code}, mAction{action} {}

  [[nodiscard]] KeyCode   getCode() const { return mCode; }
  [[nodiscard]] KeyAction getAction() const { return mAction; }

private:
  KeyCode   mCode;
  KeyAction mAction;
};

const char *toString(KeyAction action);
