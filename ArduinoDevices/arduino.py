# Copyright 2020 AngrySoft Sebastian Zwierzchowski
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from pyxiaomi.protocol import Watcher, WatcherBaseDriver
from serial import Serial, SerialException
from serial.tools.list_ports import comports
import json
from sys import stderr
from os.path import exists
from time import sleep

class ArduinoWatcher(WatcherBaseDriver):
    def __init__(self, port=None, baudrate=115200):
        self.arduino = Serial()
        self.stopping = False
        if self.port is None:
            self.port = self._detect_port()
        self.baudrate = baudrate
        self.timeout = 0
        self._connect()
        
    @staticmethod
    def _detect_port():
        port = None
        for p in comports():
            manufacturer = p.manufacturer
            if type(manufacturer) is str and 'arduino' in manufacturer.lower():
                port = p.device
        return port
    
    def _connect(self):
        while not self.arduino.is_open:
            if self.stopping:
                break
            self.arduino.port = self.port
            self.arduino.baudrate = self.baudrate
            self.arduino.timeout = self.timeout
            if self.port is None:
                return
            if self.arduino.port and exists(self.port):
                self.arduino.open()
                print(f'arduino connected: {self.port}')
            else:
                sleep(3)

    def watch(self, handler):
        while not self.stopping:
            try:
                b = self.arduino.read().decode()
                data = ''
                while not b == '\n':
                    data += b
                    b = self.arduino.read().decode()
                handler(json.loads(data))
            except json.JSONDecodeError as er:
                stderr.write(f'{er}, : {data}\n')
            except SerialException:
                pass

    def serial_write(self, value):
        try:
            msg = f'{value}\n'
            self.arduino.write(msg.encode())
        except json.JSONDecodeError as er:
            stderr.write(f'{er}, : {value}\n')
        except SerialException as err:
            self.daemon.logger.error(err)
    
    def stop(self):
        self.stopping = True
        self.arduino.close()


class ArduinoController:
    def __init__(self,port=None, baudrate=115200):
        self.name = 'Arduino'
        self._data = {}
        self._subdevices = dict()
        self.watcher = Watcher(ArduinoWatcher(port, baudrate))
        self.watcher.add_report_handler(self._handle_events)
    
    def register_sub_device(self, dev):
        self._subdevices[dev.sid] = dev
    
    def unregister_sub_device(self, sid):
        del self._subdevices[sid]
        
    def _handle_events(self, event):
        dev = self._subdevices.get(event.get('sid'))
        if dev:
            dev.report(event)
    
    def write_device(self, cmd):
        self.watcher.serial_write(cmd)


class ArduinoDevice:
    def __init__(self, sid, controller:ArduinoController):
        self.controller = controller
        self.sid = sid
        self._data = dict()
        self.controller.register_sub_device(self)
        self.writable = False
    
    def device_status(self):
        return self._data.copy()
    
    @property
    def model(self):
        return self._data.get('model', 'unknown')
        
    def report(self, data:dict) -> None:
        if 'data' in data:
            data = data['data']
        self._data.update(data)
    
        
class KelvinTable:
    # https://andi-siess.de/rgb-to-color-temperature/
    _ct = list(range(1600, 6500, 100))
    _kelvin_table = (
        {'red': '255', 'green': '115', 'blue': '0'},
        {'red': '255', 'green': '121', 'blue': '0'},
        {'red': '255', 'green': '126', 'blue': '0'},
        {'red': '255', 'green': '131', 'blue': '0'},
        {'red': '255', 'green': '138', 'blue': '18'},
        {'red': '255', 'green': '142', 'blue': '33'},
        {'red': '255', 'green': '147', 'blue': '44'},
        {'red': '255', 'green': '152', 'blue': '54'},
        {'red': '255', 'green': '157', 'blue': '63'},
        {'red': '255', 'green': '161', 'blue': '72'},
        {'red': '255', 'green': '165', 'blue': '79'},
        {'red': '255', 'green': '169', 'blue': '87'},
        {'red': '255', 'green': '173', 'blue': '94'},
        {'red': '255', 'green': '177', 'blue': '101'},
        {'red': '255', 'green': '180', 'blue': '107'},
        {'red': '255', 'green': '184', 'blue': '114'},
        {'red': '255', 'green': '187', 'blue': '120'},
        {'red': '255', 'green': '190', 'blue': '126'},
        {'red': '255', 'green': '193', 'blue': '132'},
        {'red': '255', 'green': '196', 'blue': '137'},
        {'red': '255', 'green': '199', 'blue': '143'},
        {'red': '255', 'green': '201', 'blue': '148'},
        {'red': '255', 'green': '204', 'blue': '153'},
        {'red': '255', 'green': '206', 'blue': '159'},
        {'red': '255', 'green': '209', 'blue': '163'},
        {'red': '255', 'green': '211', 'blue': '168'},
        {'red': '255', 'green': '213', 'blue': '173'},
        {'red': '255', 'green': '215', 'blue': '177'},
        {'red': '255', 'green': '217', 'blue': '182'},
        {'red': '255', 'green': '219', 'blue': '186'},
        {'red': '255', 'green': '221', 'blue': '190'},
        {'red': '255', 'green': '223', 'blue': '194'},
        {'red': '255', 'green': '225', 'blue': '198'},
        {'red': '255', 'green': '227', 'blue': '202'},
        {'red': '255', 'green': '228', 'blue': '206'},
        {'red': '255', 'green': '230', 'blue': '210'},
        {'red': '255', 'green': '232', 'blue': '213'},
        {'red': '255', 'green': '233', 'blue': '217'},
        {'red': '255', 'green': '235', 'blue': '220'},
        {'red': '255', 'green': '236', 'blue': '224'},
        {'red': '255', 'green': '238', 'blue': '227'},
        {'red': '255', 'green': '239', 'blue': '230'},
        {'red': '255', 'green': '240', 'blue': '233'},
        {'red': '255', 'green': '242', 'blue': '236'},
        {'red': '255', 'green': '243', 'blue': '239'},
        {'red': '255', 'green': '244', 'blue': '242'},
        {'red': '255', 'green': '245', 'blue': '245'},
        {'red': '255', 'green': '246', 'blue': '247'},
        {'red': '255', 'green': '248', 'blue': '251'},
        {'red': '255', 'green': '249', 'blue': '253'})
    
    def pc(self, value):
        if value % 2 and value != 0:
            value -= 1
        return self._kelvin_table[value]
    
    def ct(self, value):
        if value in self._ct:
            return self._kelvin_table[self._ct.index(value)]       

class RgbStrip(ArduinoDevice):
    def __init__(self, sid):
        super().__init__(sid)
        self.writeable = True
        self._data['model'] = 'rgbstrip'
        self.kelvin_table = KelvinTable()
        self.cmd = {'set_power': self.set_power,
                    'set_rgb': self.set_rgb,
                    'set_ct_abx': self.set_ct_abx,
                    'set_ct_pc': self.set_ct_pc,
                    'set_bright': self.bright}
        
    def write(self, data):
        _data = data.get('data', {}).copy()
        if not _data:
            raise ValueError('data is empty')
            return
        c, v = _data.popitem()
        if type(v) == dict:
            self.cmd.get(c, self._unknown)(**v)
        else:
            self.cmd.get(c, self._unknown)(v)
            
    def unknown(self, *args):
        pass
    
    def set_power(self, status):
        {'on': self.on, 'off': self.off}.get(status, self.unknown)()

    def off(self):
        self.controller.write_device('P.0')

    def on(self):
        self.controller.write_device('P.1')

    def set_bright(self, value):
        self.controller.write_device(f'B.{value}')
            
    def set_ct_abx(self, ct):
        rgb = self.kelvin_table.ct(ct)
        self.set_rgb(rgb)
        self._data.update({'ct': ct})
    
    def set_ct_pc(self, pc):
        rgb = self.kelvin_table.pc(pc)
        self.set_rgb(rgb)
        self._data.update({'ct_pc': ct_pc})
    
    def set_rgb(self, data):
        r = int(data['red'])
        g = int(data['green'])
        b = int (data['blue'])
        rgb = (r << 16) + (g << 8) + b;        
        self.controller.write_device(f'C.{rgb}')
        self._data.update(data)
    
    @property
    def power(self):
        return self._data.get('power', 'unknown')
    
    @property
    def bright(self):
        return self._data.get('bright', -1)
    
    @property
    def red(self):
        return self._data.get('red')
    
    @property
    def green(self):
        return self._data.get('green')
    
    @property
    def blue(self):
        return self._data.get('blue')
    
    @property
    def ct_pc(self):
        return self._data.get('ct_pc')
    
    @property
    def ct(self):
        return self._data.get('ct')
    
    
    def device_status(self):
        return {'power': self.power, 'red': self.red, 'green': self.green, 'blue': self.blue, 'ct_pc': self.ct_pc}
    
    class DallasTemp(ArduinoDevice):
        def __init__(self, sid, controller):
            super().__init__(sid)
            self._data['model'] = 'dallastemp'
        
        @property
        def model(self):
            return self._data.get('model')
        
        @property
        def temp(self):
            return self._data.get('temp')