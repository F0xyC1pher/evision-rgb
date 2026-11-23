# Поддержка 320F:5055 в OpenRGB

Стоковый OpenRGB не знает PID `320F:5055`. Здесь лежит минимальный патч
драйвера EVision V2: детектор `320F:5055`, карта на 128 светодиодов, имена
клавиш и матрица — всё считано с самой клавиатуры командой `0x07`.

Ключевая особенность: драйвер работает через **libusb**, не через `usbhid`.
Поэтому не нужно отключать udev-правило `99-evision-kbd-fix` из корня
репозитория: интерфейс 1 остаётся отвязанным, не создаёт фейковые события
клавиатуры и не вызывает USB-ресетов. Minecraft и Bluetooth работают нормально.

## Сборка

Нужны `qt6-base`, `hidapi`, `libusb`, `mbedtls3`, `qmake6`, `make`, `g++`.

```sh
chmod +x build.sh
./build.sh              # клонирует OpenRGB в ~/openrgb-custom и соберёт
sudo install -m 755 ~/openrgb-custom/openrgb /usr/local/bin/openrgb-evision
openrgb-evision --list-devices
# 0: EVision RGB Keyboard 320F:5055 (libusb)
```

## Использование

```sh
openrgb-evision                    # GUI со всеми плагинами
openrgb-evision --server           # GUI + SDK-сервер для MineLights и т.п.
openrgb-evision -d 0 -m Static -c FF0000
```

Проверено: `Static` применяется, ресетов интерфейса и Bluetooth нет,
Фейковые клавиатурные события от второго интерфейса не создаются.

## Что изменено в OpenRGB

- `EVisionV2KeyboardControllerDetect.cpp` — детектор `320F:5055` (hidraw и libusb).
- `EVisionV2KeyboardController.{h,cpp}` — конструктор с `hidapi_wrapper`,
  все HID-вызовы идут через него; карта и матрица из `Evision5055Layout.h`.
- `hidapi_wrapper.h`, `DetectionManager.cpp` — добавлен `hid_read` в wrapper.
- `RGBController_EVisionV2Keyboard.cpp` — 128 LED и имена для новой раскладки.
