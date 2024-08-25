import asyncio
import aioble
import bluetooth

import re
import binascii

_SCAN_UUID    = bluetooth.UUID("03b80e5a-ede8-4b33-a751-6ce34ec4c700")
_SERVICE_UUID = bluetooth.UUID(0xffc8)
_SEND_UUID    = bluetooth.UUID(0xffc9)
_RECEIVE_UUID = bluetooth.UUID(0xffca)


async def find_sparkx():
    async with aioble.scan(5000, interval_us=30000, window_us=30000, active=True) as scanner:
        async for result in scanner:
            if  _SCAN_UUID in result.services():
                print("Found Spark Control X")
                return result.device
    return None

async def main():
    device = await find_sparkx()
    if not device:
        print("Spark Control X not found")
        return
    try:
        print("Connecting to", device)
        connection = await device.connect()
        print("Connected to", device)
    except asyncio.TimeoutError:
        print("Timeout during connection")
        return

    async with connection:
        try:
            sparkx_service = await connection.service(_SERVICE_UUID)
            sparkx_receive = await sparkx_service.characteristic(_RECEIVE_UUID)
            sparkx_send    = await sparkx_service.characteristic(_SEND_UUID)
        except asyncio.TimeoutError:
            print("Timeout discovering services/characteristics")
            return

        await sparkx_receive.subscribe(notify = True)
        while connection.is_connected():
            #v = await sparkx_receive_characteristic.read()
            #print(v)
            v = await sparkx_receive.notified()
            #v = await sparkx_receive.indicated()
            print("Notified", v.hex(), v)
            if v[0] == 0x03:
                await sparkx_send.write(b'\x12\x00\x00\x00\x01')
                print("Send x12 message")
                v = await sparkx_receive.read()
                print("Read", v.hex(), v)
            await asyncio.sleep_ms(100)


asyncio.run(main())
