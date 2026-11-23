# Поддержка 320F:5055 в OpenRGB

Стоковый OpenRGB не знает PID `320F:5055`: в его таблицах есть другие модели
Evision, но не эта. Здесь лежит минимальный патч драйвера `EVision V2`:
добавлен детектор `320F:5055`, карта из 128 светодиодов, имена клавиш и
матрица, считанные из самой клавиатуры командой `0x07`.

## Сборка

Нужны `qt6-base`, `hidapi`, `libusb`, `mbedtls3`, `qmake6`, `make`, `g++`.

```sh
chmod +x build.sh
./build.sh              # клонирует OpenRGB в ~/openrgb-custom и соберёт
```

Готовый бинарник можно поставить отдельно, не заменяя пакетный OpenRGB:

```sh
sudo install -m 755 ~/openrgb-custom/openrgb /usr/local/bin/openrgb-evision
openrgb-evision --list-devices
```

## Использование

Чтобы OpenRGB увидел клавиатуру, второй интерфейс должен быть доступен как
hidraw — то есть udev-правило из корня этого репозитория (`99-evision-kbd-fix`)
должно быть отключено, а сам интерфейс — под драйвером `usbhid`.

```sh
# одна команда для переключения в режим OpenRGB
sudo systemctl --user stop evision-sdk.service
sudo mv /etc/udev/rules.d/99-evision-kbd-fix.rules ~/.local/share/evision-rgb-backup/
sudo udevadm control --reload-rules
echo -n "X-Y:1.1" | sudo tee /sys/bus/usb/drivers/usbhid/bind
```

Путь интерфейса смотри в `ls /sys/bus/usb/devices/*:1.1`.

Для доступа пользователя к hidraw используется правило
`60-openrgb-evision.rules` (входит в установку этого патча через `build.sh`
не ставится — установи его вручную):

```
SUBSYSTEM=="hidraw", ATTRS{idVendor}=="320f", ATTRS{idProduct}=="5055", TAG+="uaccess"
```

## Что работает

- аппаратные режимы (`Static`, `Breathing` и др.) и плагины OpenRGB;
- per-key цвета через Direct-режим при работающем SDK-сервере OpenRGB.

Ограничение: при отключённом udev-правиле возвращается исходная проблема с
USB-ресетами интерфейса 1 и Bluetooth-адаптера на том же хабе.
