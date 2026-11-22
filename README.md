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

# Per-key RGB БЕЗ демона — записывается во flash клавиатуры
evision-rgb keys                            # все имена клавиш -> слоты
evision-rgb custom esc=ff0000 f1=00ff00 1=0000ff
evision-rgb rainbow                         # радуга по всем клавишам

# Per-key "на лету" (пока процесс жив, для реакций на события игр)
evision-rgb key esc=ff0000 f1=00ff00 1=0000ff
evision-rgb slots 0=ff0000 8=00ff00 9=0000ff 30
```

`custom` / `rainbow` пишут цвета в энергонезависимую память клавиатуры и
переключают профиль в режим `custom` (0x14) — **никакой процесс не нужен**,
цвета сохраняются после выхода команды и после перезагрузки.

`key` / `slots` работают в volatile direct-режиме: прошивка сбрасывает его
через ~1 с, поэтому процесс повторно отправляет полный кадр каждые ~0.2 с.
`Ctrl-C` корректно возвращает клавиатуру в аппаратный режим.

Для игр (Minecraft, Terraria): моды/скрипты могут вызывать `evision-rgb
static ...`, `effect ...` или `custom ...` — все эти команды завершаются
сразу, демон не требуется.

Доступные режимы (`effect`): `wave-short`, `wave-long`, `wheel`, `spectrum`,
`breathing`, `static`, `reactive`, `reactive-ripple`, `reactive-line`,
`starlight-fast`, `blooming`, `rainbow-vertical`, `hurricane`, `accumulate`,
`starlight-slow`, `visor`, `surmount`, `rainbow-circle`, `custom`.

## MineLights (Minecraft)

[MineLights](https://modrinth.com/mod/minelights) на Linux управляет подсветкой
через OpenRGB SDK на `127.0.0.1:6742`. Стоковый OpenRGB нашу клавиатуру не
видит, поэтому в репозитории есть мост `evision-sdk`: он поднимает SDK-сервер,
представляет клавиатуру как обычный 128-LED OpenRGB-контроллер и переводит
обновления LED в HID-кадры.

```sh
# 1. Убедись, что настоящий OpenRGB не запущен (порт 6742 должен быть свободен)
# 2. Запусти мост
evision-sdk                      # или: systemctl --user enable --now evision-sdk

# 3. В MineLights: Mods > MineLights > Config -> включи OpenRGB integration
#    и нажми Refresh devices
```

Мост отдаёт клавиатуру с именами LED `Esc`, `F1`…`F12`, `1`…`0`, `Q`… и
`Num0`…`Num9`, так что эффекты MineLights (health bar на F1-F4, компас на
numpad, биомы, погода, статусы) работают без дополнительной настройки.
Задержка обновления — ~10 Гц (`--hz`, можно поднять).

Для проверки моста без MineLights:

```sh
python3 - <<'PY'
import socket, struct, time
s = socket.create_connection(('127.0.0.1', 6742))
def send(pid, data=b''):
    s.sendall(b'ORGB' + struct.pack('<III', 0, pid, len(data)) + data)
send(40, struct.pack('<I', 4))       # protocol version
send(0)                              # controller count
cols = [0]*128
cols[0] = 0x0000ff                   # Esc red (RGBColor = 0x00BBGGRR)
cols[8] = 0x00ff00                   # F1 green
cols[9] = 0xff0000                   # 1 blue
p = struct.pack('<I', 128) + b''.join(struct.pack('<I', c) for c in cols)
for _ in range(100):
    send(1050, p); time.sleep(0.1)
PY
```

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
