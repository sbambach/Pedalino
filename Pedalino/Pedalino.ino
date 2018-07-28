//#define DEBUG_PEDALINO

#include "Pedalino.h"
#include "Config.h"
#include "MIDIRouting.h"
#include "Controller.h"
#include "ControlChange.h"
#include "NoteNumbers.h"
#include "Display.h"

// Standard setup() and loop()

void setup(void)
{
#ifdef DEBUG_PEDALINO
  SERIALDEBUG.begin(115200);
#endif
  read_eeprom();

  // Initiate serial MIDI communications, listen to all channels and turn Thru off
  //midi_routing_start();
#ifndef DEBUG_PEDALINO
  USB_MIDI.begin(MIDI_CHANNEL_OMNI);
  interfaces[PED_USBMIDI].midiThru ? USB_MIDI.turnThruOn() : USB_MIDI.turnThruOff();
#endif
  DIN_MIDI.begin(MIDI_CHANNEL_OMNI);
  interfaces[PED_LEGACYMIDI].midiThru ? DIN_MIDI.turnThruOn() : DIN_MIDI.turnThruOff();
  RTP_MIDI.begin(MIDI_CHANNEL_OMNI);
  interfaces[PED_APPLEMIDI].midiThru ? RTP_MIDI.turnThruOn() : RTP_MIDI.turnThruOff();

  autosensing_setup();
  controller_setup();
  //midi_clock_setup();

  pinMode(LCD_BACKLIGHT, OUTPUT);
  analogWrite(LCD_BACKLIGHT, backlight);

  irrecv.enableIRIn();                        // Start the IR receiver
  irrecv.blink13(true);
  bluetooth.begin(9600);                      // Start the Bluetooth receiver
  bluetooth.println("AT+NAME=Pedalino(TM)");  // Set bluetooth device name

  display(MD_Menu::DISP_INIT);
  M.begin();
  M.setMenuWrap(true);
  M.setAutoStart(AUTO_START);
  //M.setTimeout(MENU_TIMEOUT);
}


void loop(void)
{
  static bool prevMenuRun = true;

  // Detect if we need to initiate running normal user code
  if (prevMenuRun && !M.isInMenu())
    screen_update(true);
  prevMenuRun = M.isInMenu();

  // If we are not running and not autostart
  // check if there is a reason to start the menu
  if (!M.isInMenu() && !AUTO_START)
  {
    uint16_t dummy;

    if (navigation(dummy) == MD_Menu::NAV_SEL)
      M.runMenu(true);
  }
  if (!M.isInMenu()) screen_update();
  else lcd.noCursor();

  M.runMenu();   // just run the menu code

  // Check whether the input has changed since last time, if so, send the new value over MIDI
  midi_refresh();
  midi_routing();
}

