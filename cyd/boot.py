'''Micropython ILI9341 with xpt2046 touch screen demo for CYD
    libraries and boilerplate altered from @rdagger ili9341 repo
    https://raw.githubusercontent.com/rdagger/micropython-ili9341
'''

from ili9341 import Display, color565
from xpt2046 import Touch
from machine import idle, Pin, SPI
import network
import time
from umqtt.simple import MQTTClient
import json
#import tt24
import ntptime


spi1 = SPI(1, baudrate=40000000, sck=Pin(14), mosi=Pin(13))
xdisplay = Display(spi1, dc=Pin(2), cs=Pin(15), rst=Pin(0))
    
def sub_cb(topic, msg):
    j_msg = json.loads(msg)
    print((topic, msg))
    n=0
    #font = tt24
    now = time.localtime(time.time() + (-8 * 3600))
    if int(now[3]) >12:
        hr = str(int(now[3])-12)
        ampm = "PM"
    elif int(now[3])==0:
        hr = "12"
        ampm = "AM"
    else:
        hr = now[3]
        ampm = "AM"
    if int(now[4])<10:
        min = "0"+str(now[4])
    else:
        min = str(now[4])
    line = str(hr)+":"+min+ampm
    xdisplay.draw_text8x8(230-n*10,5,line, Demo.WHITE,background=color565(0, 0, 0),rotate=90)
    n=1
    for item in j_msg:
        if item != "dateTime":
            print (item,":",j_msg[item])
            line = str(item)+":"+str(j_msg[item])
            xdisplay.draw_text8x8(230-n*10,5,line, Demo.WHITE,background=color565(0, 0, 0),rotate=90)
#           xdisplay.draw_text(230-n*10,5,line, font, Demo.WHITE,background=color565(0, 0, 0),landscape=True)
            n=n+1
            
class Demo(object):
    '''Touchscreen simple demo.'''
    CYAN = color565(0, 255, 255)
    PURPLE = color565(255, 0, 255)
    WHITE = color565(255, 255, 255)

    def __init__(self, display, spi2):
        print("getting started")
        self.display = display
        self.touch = Touch(spi2, cs=Pin(33), int_pin=Pin(36),
                           int_handler=self.touchscreen_press)
        sta_if = network.WLAN(network.STA_IF)
        sta_if.active(True)
        sta_if.connect('Quinn and Cole', 'ClevelandLulu')
        while not sta_if.isconnected():
            pass
        print('network config:', sta_if.ifconfig())

        c = MQTTClient("cyd", '10.0.0.11',port=1883)
        c.set_callback(sub_cb)
        c.connect()
        c.subscribe(b"weather/data")
        ntptime.settime() #set the system time
        while True:
            if True:
                # Blocking wait for message
                c.wait_msg()
        else:
            # Non-blocking wait for message
            c.check_msg()
            # Then need to sleep to avoid 100% CPU usage (in a real
            # app other useful actions would be performed instead)
            time.sleep(1)

        c.disconnect()

        
        '''Initialize box.

        Args:
            display (ILI9341): display object
            spi2 (SPI): SPI bus
        '''
        self.display = display
        self.touch = Touch(spi2, cs=Pin(33), int_pin=Pin(36),
                           int_handler=self.touchscreen_press)
        # Display initial message
        self.display.draw_text8x8(self.display.width // 2 - 32,
                                  self.display.height - 9,
                                  "TOUCH ME",
                                  self.WHITE,
                                  background=self.PURPLE)

        # A small 5x5 sprite for the dot
        self.dot = bytearray(b'\x00\x00\x07\xE0\xF8\x00\x07\xE0\x00\x00\x07\xE0\xF8\x00\xF8\x00\xF8\x00\x07\xE0\xF8\x00\xF8\x00\xF8\x00\xF8\x00\xF8\x00\x07\xE0\xF8\x00\xF8\x00\xF8\x00\x07\xE0\x00\x00\x07\xE0\xF8\x00\x07\xE0\x00\x00')

    def touchscreen_press(self, x, y):
        '''Process touchscreen press events.'''
        print("Display touched.")
        
        # Y needs to be flipped
        y = (self.display.height - 1) - y
        # Display coordinates
        self.display.draw_text8x8(self.display.width // 2 - 32,
                                  self.display.height - 9,
                                  "{0:03d}, {1:03d}".format(x, y),
                                  self.CYAN)
        # Draw dot
        self.display.draw_sprite(self.dot, x - 2, y - 2, 5, 5)


def test():
    
    '''
    Display Pins:
    IO2 	TFT_RS 	AKA: TFT_DC
    IO12 	TFT_SDO 	AKA: TFT_MISO
    IO13 	TFT_SDI 	AKA: TFT_MOSI
    IO14 	TFT_SCK 	
    IO15 	TFT_CS 	
    IO21 	TFT_BL

    Touch Screen Pins:
    IO25 	XPT2046_CLK 	
    IO32 	XPT2046_MOSI 	
    IO33 	XPT2046_CS 	
    IO36 	XPT2046_IRQ 	
    IO39 	XPT2046_MISO
    '''
    
    
    ''' Set up the display - ili9341
        Baud rate of 40000000 seems about the max '''
    #spi1 = SPI(1, baudrate=40000000, sck=Pin(14), mosi=Pin(13))
    #display = Display(spi1, dc=Pin(2), cs=Pin(15), rst=Pin(0))
    
    
    bl_pin = Pin(21, Pin.OUT)
    bl_pin.on()
    
    # Set up the touch screen digitizer - xpt2046
    spi2 = SPI(2, baudrate=1000000, sck=Pin(25), mosi=Pin(32), miso=Pin(39))

    Demo(xdisplay, spi2)

    try:
        while True:
            idle()

    except KeyboardInterrupt:
        print("\nCtrl-C pressed.  Cleaning up and exiting...")
    finally:
        display.cleanup()


test()