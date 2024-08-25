import bluetooth
import time
from ble_advertising import decode_services, decode_name, decode_manuf, dump_all
from micropython import const
import random

from time import sleep

_IRQ_CENTRAL_CONNECT = const(1)
_IRQ_CENTRAL_DISCONNECT = const(2)
_IRQ_GATTS_WRITE = const(3)
_IRQ_GATTS_READ_REQUEST = const(4)
_IRQ_SCAN_RESULT = const(5)
_IRQ_SCAN_DONE = const(6)
_IRQ_PERIPHERAL_CONNECT = const(7)
_IRQ_PERIPHERAL_DISCONNECT = const(8)
_IRQ_GATTC_SERVICE_RESULT = const(9)
_IRQ_GATTC_SERVICE_DONE = const(10)
_IRQ_GATTC_CHARACTERISTIC_RESULT = const(11)
_IRQ_GATTC_CHARACTERISTIC_DONE = const(12)
_IRQ_GATTC_DESCRIPTOR_RESULT = const(13)
_IRQ_GATTC_DESCRIPTOR_DONE = const(14)
_IRQ_GATTC_READ_RESULT = const(15)
_IRQ_GATTC_READ_DONE = const(16)
_IRQ_GATTC_WRITE_DONE = const(17)
_IRQ_GATTC_NOTIFY = const(18)
_IRQ_GATTC_INDICATE = const(19)

_ADV_IND = const(0x00)
_ADV_DIRECT_IND = const(0x01)
_ADV_SCAN_IND = const(0x02)
_ADV_NONCONN_IND = const(0x03)
_ADV_SCAN_RSP = const(0x04)

#MIDI_SERVICE_UUID = bluetooth.UUID(0xFFC0)
#MIDI_TX_CHAR_UUID = bluetooth.UUID(0xFFC1)
#MIDI_RX_CHAR_UUID = bluetooth.UUID(0xFFC2)


SERVICE_UUID = bluetooth.UUID(0xFFC8)
TX_CHAR_UUID = bluetooth.UUID(0xFFC9)
RX_CHAR_UUID = bluetooth.UUID(0xFFCA)

class SparkX:
    def __init__(self, ble):
        self._ble = ble
        self._ble.active(True)
        self._ble.irq(self._irq)
        self._reset()


    def _reset(self):
        # Cached name and address from a successful scan.
        self._name = None
        self._addr_type = None
        self._addr = None

        # In a current scan
        self._in_scan = False
        
        # Cached value (if we have one)
        self._value = None

        self._found ={}
        
        # Callbacks for completion of various operations.
        # These reset back to None after being invoked.
        self._scan_callback = None
        self._conn_callback = None
        self._read_callback = None

        # Persistent callback for when new data is notified from the device.
        self._notify_callback = None

        # Connected device.
        self._conn_handle = None
        self._start_handle = None
        self._end_handle = None
        
        self._rx_value_handle = None
        self._notify_dsc_handle  = None
        self._tx_value_handle = None

    def _irq(self, event, data):
        if event == _IRQ_SCAN_RESULT:
            addr_type, addr, adv_type, rssi, adv_data = data
            
            print(addr_type, bytes(addr).hex(), adv_type, rssi, bytes(adv_data).hex(), end=" : ")
            dump_all(bytes(adv_data))
            if adv_type  in (_ADV_IND, _ADV_DIRECT_IND, _ADV_SCAN_RSP):
                type_list = decode_services(adv_data)
                name = decode_name(adv_data)
                
                #if (MIDI_SERVICE_UUID in type_list) or (name == "Spark MINI BLE"):
                if name[:5] == "Spark":
                    self._addr_type = addr_type
                    self._addr = bytes(addr)  # Note: addr buffer is owned by caller so need to copy it.
                    self._name = "SparkX"
                    self._ble.gap_scan(None)  # Stop scan

        elif event == _IRQ_SCAN_DONE:
            # Scan is finished
            self._in_scan = False


        elif event == _IRQ_PERIPHERAL_CONNECT:
            # Connect successful so start dscovering services
            conn_handle, addr_type, addr = data
            
            if addr_type == self._addr_type and addr == self._addr:
                self._conn_handle = conn_handle
                self._ble.gattc_discover_services(self._conn_handle)

        elif event == _IRQ_PERIPHERAL_DISCONNECT:
            # Disconnect (either initiated by us or the remote end).
            conn_handle, _, _ = data
            
            if conn_handle == self._conn_handle:
                # If it was initiated by us, it'll already be reset.
                self._reset()


        elif event == _IRQ_GATTC_SERVICE_RESULT:
            # Connected device returned a service.
            conn_handle, start_handle, end_handle, uuid = data
            
            print("Service", conn_handle, start_handle, end_handle, uuid,)
            self._found[start_handle] = (uuid, start_handle, end_handle)
            if conn_handle == self._conn_handle and uuid == SERVICE_UUID:
                self._start_handle, self._end_handle = start_handle, end_handle



        elif event == _IRQ_GATTC_SERVICE_DONE:
            # Service query complete.
            if self._start_handle and self._end_handle:
                self._ble.gattc_discover_characteristics(self._conn_handle, self._start_handle, self._end_handle)
            else:
                print("Failed to find service")


        elif event == _IRQ_GATTC_CHARACTERISTIC_RESULT:
            # Connected device returned a characteristic.
            conn_handle, def_handle, value_handle, properties, uuid = data

            print("Characteristic", conn_handle, def_handle, value_handle, properties, uuid)
            if conn_handle == self._conn_handle and uuid == RX_CHAR_UUID:
                self._rx_value_handle = value_handle
                print("Got rx handle", value_handle)
            if conn_handle == self._conn_handle and uuid == TX_CHAR_UUID:
                self._tx_value_handle = value_handle
                print("Got tx handle", value_handle)
                
        elif event == _IRQ_GATTC_CHARACTERISTIC_DONE:
            # Characteristic query complete.
            if self._rx_value_handle:
                self._ble.gattc_discover_descriptors(self._conn_handle, self._start_handle, self._end_handle)
            else:
                print("Failed to find rx characteristic")
            
        elif event == _IRQ_GATTC_DESCRIPTOR_RESULT:
            # Called for each descriptor found by gattc_discover_descriptors().
            conn_handle, dsc_handle, uuid = data
            
            print("Descriptor", conn_handle, dsc_handle, uuid)
            if uuid == bluetooth.UUID(0x2902):
                self._notify_dsc_handle = dsc_handle
                print("Notify handle ", dsc_handle)
 
        elif event == _IRQ_GATTC_DESCRIPTOR_DONE:
            # Called once service discovery is complete.
            # Note: Status will be zero on success, implementation-specific value otherwise.
            conn_handle, status = data
            
            if self._notify_dsc_handle:
                self._ble.gattc_write(self._conn_handle, self._notify_dsc_handle, b'\x01\x00', 1)
                print("Registered for notify on handle", self._notify_dsc_handle)
            else:
                print("Failed to find descriptor")
                

        elif event == _IRQ_GATTC_READ_RESULT:
            # A read completed successfully.
            conn_handle, value_handle, char_data = data
            
            if conn_handle == self._conn_handle and value_handle == self._rx_value_handle:
                self._update_value(char_data)
                if self._read_callback:
                    self._read_callback(self._value)
                    self._read_callback = None

        elif event == _IRQ_GATTC_READ_DONE:
            # Read completed (no-op).
            conn_handle, value_handle, status = data


        elif event == _IRQ_GATTC_NOTIFY:
            conn_handle, value_handle, notify_data = data
            print("Event - notified {}".format(bytes(notify_data).hex()))
            if conn_handle == self._conn_handle and value_handle == self._rx_value_handle:
                self._update_value(notify_data)
                if self._notify_callback:
                    self._notify_callback(self._value)
                    
    # Returns true if the scan was successful
    def scan_success(self):
        return self._addr_type is not None

    def in_scan(self):
        return self._in_scan
    
    # Returns true if we've successfully connected and discovered characteristics.
    def is_connected(self):
        return self._conn_handle is not None and self._rx_value_handle is not None

    # Find a device advertising the environmental sensor service.
    def scan(self, callback=None):
        self._addr_type = None
        self._addr = None
        self._in_scan = True
        self._ble.gap_scan(2000, 30000, 30000, True)

    # Connect to the specified device (otherwise use cached address from a scan).
    def connect(self, addr_type=None, addr=None):
        self._addr_type = addr_type or self._addr_type
        self._addr = addr or self._addr
        if self._addr_type is None or self._addr is None:
            return False
        self._ble.gap_connect(self._addr_type, self._addr)
        return True

    # Disconnect from current device.
    def disconnect(self):
        if not self._conn_handle:
            return
        self._ble.gap_disconnect(self._conn_handle)
        self._reset()

    # Issues an (asynchronous) read, will invoke callback with data.
    def read(self, callback):
        if not self.is_connected():
            return
        self._read_callback = callback
        try:
            self._ble.gattc_read(self._conn_handle, self._rx_value_handle)
        except OSError as error:
            print(error)

    # Sets a callback to be invoked when the device notifies us.
    def on_notify(self, callback):
        self._notify_callback = callback

    def _update_value(self, data):
        self._value = bytes(data)

    def value(self):
        return self._value
    
    def write(self, data):
        if not self.is_connected():
            return
        self._ble.gattc_write(self._conn_handle, self._tx_value_handle, data)


def read_callback(result):
    print("Read value {}".format(result.hex()))

bank_change = False
new_bank = False
bank = 0

def notified_callback(result):
    global send_once, bank_change, new_bank, bank
    print("Notified value {}".format(result.hex()))
    if result[0] == 0x03 and (result[4] == 0xfd or result[4] == 0xfe):
        bank_change = True
    if result[0] == 0x08:
        new_bank = True
        bank = result[4]    
    
def make_sparkx_lights(light, red, green, blue):
    return bytes.fromhex("0100000001") + bytes([light]) + bytes.fromhex("01FF00") + bytes([blue]) + bytes([green]) + bytes([red]) + bytes([0])

def write(dat):
    print("Sending {}".format(dat.hex()))
    central.write(dat)   
    
def demo(ble, central):
    global send_once, bank_change, new_bank, bank

    send1  = bytes.fromhex("0b0000000000")
    send2  = bytes.fromhex("0d0000000000")
    send3  = bytes.fromhex("08000000")
    send4  = bytes.fromhex("01000000010702100000000000")
    send5  = bytes.fromhex("01000000010700100000000000")
    send6  = bytes.fromhex("1400000001")
    
    sends = [send1, send2, send3, send4, send5, send6]

    while not central.scan_success():
        if not central.in_scan():
            central.scan()
        
    central.connect()
    while not central.is_connected():
        time.sleep_ms(100)

    print("Connected")
    print()
    
    central.on_notify(notified_callback)

    time.sleep_ms(1000)

    cmd = bytes.fromhex("1400000002")
    write(cmd)     

    time.sleep_ms(1000)
    central.read(read_callback)

    cmd = bytes.fromhex("1200000002")
    write(cmd)     

    time.sleep_ms(1000)
    central.read(read_callback)

    while central.is_connected() and sends:
        time.sleep_ms(500)
        dat = sends[0]
        if dat[0] == 0x01:
            dat = dat[:4] + bytes([bank]) + dat[5:]
        if dat[0] == 0x14:
            dat = dat[:4] + bytes([bank]) 
        sends.pop(0)
        write(dat)
        
    print("Finsihed setup")

    for i in range(0,6):
        time.sleep_ms(300)
        #dat = bytes.fromhex("0100000001")+bytes([i])+bytes.fromhex("01ff00ffff00") 
        dat = bytes.fromhex("0100000001")+bytes([i])+bytes.fromhex("010000000000") 
        write(dat)

    while central.is_connected():
        if bank_change:
            print("BANK CHANGE")
            bank_change = False
            cmd = bytes.fromhex("08000000")
            write(cmd)
        if new_bank:
            cmd = bytes.fromhex("14000000") + bytes([bank])
            write(cmd)
            time.sleep_ms(1000)
            cmd = bytes.fromhex("13000000") + bytes([bank]) + b'Paul!!' + b'\x00'
            time.sleep_ms(1000)
            write(cmd)                  
            cmd = bytes.fromhex("12000000") + bytes([bank])
            write(cmd)            
            new_bank = False
        
    print("Disconnected")

if __name__ == "__main__":
    ble = bluetooth.BLE()
    central = SparkX(ble)
    demo(ble, central)

