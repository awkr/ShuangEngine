#pragma once

enum class InputEventSource { NONE, KEYBOARD };

class InputEvent {
public:
  InputEvent(const InputEventSource source) : mSource{source} {}

  InputEventSource getSource() const { return mSource; }

private:
  InputEventSource mSource;
};

enum class KeyCode { NONE, ESCAPE };
enum class KeyAction { NONE, DOWN, UP };

class KeyInputEvent : public InputEvent {
public:
  KeyInputEvent(KeyCode code, KeyAction action)
      : InputEvent{InputEventSource::KEYBOARD}, mCode{code}, mAction{action} {}

  KeyCode getCode() const { return mCode; }

private:
  KeyCode mCode;
  KeyAction mAction;
};
