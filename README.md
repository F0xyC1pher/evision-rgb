# evision-rgb

Программное управление подсветкой клавиатур на контроллере **Evision V2**
(`320F:5055`) в Linux — без OpenRGB и без сломанного Bluetooth.

## Зачем это нужно

У клавиатур семейства `320F:5055` (Evision / Wobkey Rainy 75 / Crush 80 /
Kreo Hive / GMK87 и др.) второй HID-интерфейс (`1-1.6:1.1`, usage page `0xFF1C`)
представляется ядром как «мышь + ещё две клавиатуры». `usbhid` начинает его
опрашивать, прошивка не справляется, и ядро уходит в бесконечный цикл
`usb X: reset full-speed USB device`. На общем USB-хабе это убивает соседний
Bluetooth-донгль (RTL8761BU): HCI-команды уходят в `tx timeout`, адаптер
переперечисляется как `hci0 → hci1 → …`.

Решение из двух частей:

1. udev-правило отвязывает **только второй интерфейс** от `usbhid` (клавиши и
   аппаратная подсветка на первом интерфейсе продолжают работать).
2. Утилита говорит напрямую со вторым интерфейсом через `libhidapi-libusb`,
   минуя `usbhid` — ресетов нет, но прошивка отвечает на команды подсветки.

## Установка

```sh
./install.sh
```

Это скопирует:

| Файл | Куда | Назначение |
|---|---|---|
| `evision-rgb` | `/usr/local/bin/evision-rgb` | CLI-утилита |
| `udev/99-evision-kbd-fix.rules` | `/etc/udev/rules.d/` | отвязка интерфейса 1 от `usbhid` |
| `udev/kbd-fix-unbind-if1` | `/usr/local/bin/` | скрипт отвязки |
| `udev/60-evision-rgb.rules` | `/etc/udev/rules.d/` | доступ юзеру к USB (uaccess) |

После установки клавиатуру нужно отключить/подключить или выполнить
`sudo udevadm control --reload-rules && sudo udevadm trigger`.

Зависимости: Python 3, `libhidapi-libusb.so.0` (Arch: `hidapi`, входит в базу).
Права root **не нужны** — доступ выдаётся через `uaccess`.

## Использование

```sh
evision-rgb status                          # текущие режим/яркость/цвет
evision-rgb static ff0000 8                 # статический цвет + яркость 0..9
evision-rgb effect breathing speed=6 color=ff00ff
evision-rgb effects                         # все режимы
evision-rgb save                            # сохранить конфиг в ~/.config/evision-rgb.json
evision-rgb restore                         # вернуть сохранённый конфиг
```

Доступные режимы (`effect`): `wave-short`, `wave-long`, `wheel`, `spectrum`,
`breathing`, `static`, `reactive`, `reactive-ripple`, `reactive-line`,
`starlight-fast`, `blooming`, `rainbow-vertical`, `hurricane`, `accumulate`,
`starlight-slow`, `visor`, `surmount`, `rainbow-circle`, `custom`.

## Протокол

64-байтовые HID-пакеты на интерфейсе 1, report ID `4`:

```
[0]    = 0x04  report ID
[1:3]  = checksum (little endian) = sum(bytes[3:64])
[3]    = cmd
[4]    = size
[5:7]  = offset (little endian)
[7]    = статус в ответе (0 = ok)
[8:]   = данные
```

Команды: `0x01` begin configure, `0x02` end configure, `0x03` capabilities
(ответ начинается `aa 55`), `0x05` read config, `0x06` write config,
`0x12` send dynamic colors, `0x13` end dynamic colors.

Параметры профиля (смещение `profile * 0x40 + 0x01`):

| Смещение | Параметр |
|---|---|
| `0x00` | режим |
| `0x01` | яркость |
| `0x02` | скорость |
| `0x03` | направление |
| `0x04` | random color flag |
| `0x05..0x07` | цвет R,G,B |

Протокол взят из реверс-инжиниринга OpenRGB (форк `huacnlee/OpenRGB`,
драйвер EVision V2, GPL-2.0).

## Проверка

После включения убедись, что счётчики ресетов не растут:

```sh
journalctl -k | grep -c "Bluetooth: hci.*Resetting usb device"
journalctl -k | grep -c "1-1.6: reset"
```

Интерфейс 1 должен быть без драйвера:

```sh
[ -e /sys/bus/usb/devices/1-1.6:1.1/driver ] && echo BOUND || echo UNBOUND
```

## Ограничения

- Per-key direct mode не реализован: нужен точный 128-слотовый LED-мап для
  конкретной ревизии клавиатуры. Статический цвет и все аппаратные эффекты
  работают без мапа.
- Прошивка на части ревизий держит EVision-интерфейс молчащим — утилита
  сообщит `no response`.
- Прошивка сохраняет настройки в энергонезависимой памяти клавиатуры.

## Лицензия

GPL-3.0 — см. [LICENSE](LICENSE).
