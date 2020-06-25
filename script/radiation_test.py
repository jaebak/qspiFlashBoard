#!/usr/bin/python3
import urwid
import datetime
import multiprocessing
import time
import os
import subprocess
import re

def tail(f, n, offset=0):
  proc = subprocess.check_output(['tail', '-n', str(n + offset), f])
  #return proc.decode('utf-8')
  proc = proc.decode('utf-8')
  proc = proc.split('\n')[:n]
  return '\n'.join(proc)

def getOutputMessage():
  if not os.path.exists('pid.'+str(pid)+'.out'): return ""
  return tail('pid.'+str(pid)+'.out', 15)

def getTotalErrors():
  # Find all lines with 'Number of errors'
  totalError = 0
  if not os.path.exists('pid.'+str(pid)+'.out'): return -1
  with open('pid.'+str(pid)+'.out') as inFile:
    for line in inFile:
      if 'Number of errors' not in line: continue
      error = [int(s) for s in line.split() if s.isdigit()][-1]
      totalError += error
  return totalError

def getTotalUsbErrors():
  # Find all lines with 'Number of errors'
  totalError = 0
  if not os.path.exists('pid.'+str(pid)+'.out'): return -1
  with open('pid.'+str(pid)+'.out') as inFile:
    for line in inFile:
      if '[ERROR]' not in line: continue
      totalError += 1
  return totalError

def getUsbStatus():
  subprocess.run(['./bin/checkUsb','pid.'+str(pid)])
  proc = tail('pid.'+str(pid)+'.out',1, 2)
  if 'USB status' not in proc: return 'Error'
  proc = [int(s) for s in proc.split() if s.isdigit()][-1]
  return str(proc)

def getPromStatus():
  subprocess.run(['./bin/checkProm', 'pid.'+str(pid)])
  proc = tail('pid.'+str(pid)+'.out',7, 2)
  totalStatus = ''
  for line in proc.split('\n'):
    if 'Prom status' not in line: return 'Error\n'
    totalStatus += re.findall(r'\d+', line)[-1]+'\n'
  return totalStatus

def menu():
  usbStatus.set_text("USB controller status (Should be 2): "+getUsbStatus()+"\n")
  promStatus.set_text("Prom status: \n"+getPromStatus())

  title = 'Radiation Test: PID='+str(pid)+'\n'
  body = [urwid.Text(title), timeStatus, usbStatus, promStatus, applicationStatus, totalErrorStatus, usbErrorStatus]
  choices = ['check USB&Prom status', 'continuousReadAndValidate', 'writeProm', 'eraseProm', 'readAndValidate', 'Exit']
  for c in choices:
    button = urwid.Button(c)
    if (c=="Exit"): urwid.connect_signal(button, 'click', exit_program)
    elif (c=="check USB&Prom status"): urwid.connect_signal(button, 'click', run_checkStatus, c)
    elif (c=="continuousReadAndValidate"): urwid.connect_signal(button, 'click', run_application, c)
    elif (c=="writeProm"): urwid.connect_signal(button, 'click', run_writeProm, c)
    elif (c=="eraseProm"): urwid.connect_signal(button, 'click', run_eraseProm, c)
    elif (c=="readAndValidate"): urwid.connect_signal(button, 'click', run_readValidate, c)
    else: urwid.connect_signal(button, 'click', item_chosen, c)
    body.append(urwid.AttrMap(button, None, focus_map='reversed'))
  body.append(outputMessage)
  return urwid.ListBox(urwid.SimpleFocusListWalker(body))

#def execute(program):
#  process = subprocess.run('./bin/'+program)

def run_checkStatus(button, choice):
  usbStatus.set_text("USB controller status (Should be 2): "+getUsbStatus()+"\n")
  promStatus.set_text("Prom status: \n"+getPromStatus())
  goto_menu(button)

def run_readValidate(button, choice):
  # Spawn application
  applications.append(subprocess.Popen(['./bin/'+choice, 'pid.'+str(pid), currentDataFile.get_text()[0]]));
  response = urwid.Text(['Running ', choice, ': PID='+str(pid)+'\n'])
  urwid.connect_signal(stopNext, 'click', stop_application, choice)
  main.original_widget = urwid.Filler(urwid.Pile([response, timeStatus, applicationStatus, totalErrorStatus, usbErrorStatus,
    urwid.AttrMap(stopNext, None, focus_map='reversed'), outputMessage]))

def run_eraseProm(button, choice):
  # Spawn application
  applications.append(subprocess.Popen(['./bin/'+choice, 'pid.'+str(pid)]));
  currentDataFile.set_text('data/empty.dat')
  response = urwid.Text(['Running ', choice, ': PID='+str(pid)+'\n'])
  urwid.connect_signal(stopNext, 'click', stop_application, choice)
  main.original_widget = urwid.Filler(urwid.Pile([response, timeStatus, applicationStatus, totalErrorStatus, usbErrorStatus,
    urwid.AttrMap(stopNext, None, focus_map='reversed'), outputMessage]))

def run_writeProm(button, choice):
  # Spawn application
  applications.append(subprocess.Popen(['./bin/'+choice, 'pid.'+str(pid)]));
  currentDataFile.set_text('data/counter.dat')
  response = urwid.Text(['Running ', choice, ': PID='+str(pid)+'\n'])
  urwid.connect_signal(stopNext, 'click', stop_application, choice)
  main.original_widget = urwid.Filler(urwid.Pile([response, timeStatus, applicationStatus, totalErrorStatus, usbErrorStatus,
    urwid.AttrMap(stopNext, None, focus_map='reversed'), outputMessage]))

def run_application(button, choice):
  # Spawn application
  applications.append(subprocess.Popen(['./bin/'+choice, 'pid.'+str(pid)]));
  response = urwid.Text(['Running ', choice, ': PID='+str(pid)+'\n'])
  urwid.connect_signal(stopNext, 'click', stop_application, choice)
  main.original_widget = urwid.Filler(urwid.Pile([response, timeStatus, applicationStatus, totalErrorStatus, usbErrorStatus,
    urwid.AttrMap(stopNext, None, focus_map='reversed'), outputMessage]))
  
def stop_application(button, choice):
  # Kill application
  if len(applications) != 0:
    application = applications.pop()
    application.kill()
    application.wait()
  response = urwid.Text([choice, ' has stopped. PID='+str(pid)+'\n'])
  done = urwid.Button(u'Ok')
  urwid.connect_signal(done, 'click', goto_menu)
  main.original_widget = urwid.Filler(urwid.Pile([response, timeStatus, applicationStatus, totalErrorStatus, usbErrorStatus,
      urwid.AttrMap(done, None, focus_map='reversed'), outputMessage]))

def goto_menu(button):
  main.original_widget = urwid.Padding(menu(), left=0, right=0)
  #main.original_widget.get_focus_widgets()
  #main_loop.get_focus_widgets()
  #main.original_widget.set_focus_path(topFocus)
  #main.original_widget.focus_position = 1

def item_chosen(button, choice):
  response = urwid.Text([u'You chose ', choice, u'\n'])
  done = urwid.Button(u'Ok')
  urwid.connect_signal(done, 'click', exit_program)
  main.original_widget = urwid.Filler(urwid.Pile([response,
      urwid.AttrMap(done, None, focus_map='reversed')]))

def exit_program(button):
  raise urwid.ExitMainLoop()

def refresh(_loop, _data):
  timeStatus.set_text("Current time: "+datetime.datetime.now().strftime("%H:%M:%S")+'\n')
  totalErrorStatus.set_text("Total validation errors: "+str(getTotalErrors())+"\n")
  usbErrorStatus.set_text("Total USB errors: "+str(getTotalUsbErrors())+"\n")

  if len(applications) != 0:
    poll = applications[0].poll()
    if poll == None:
      applicationStatus.set_text("Application running\n")
      stopNext.set_label('Stop')
    else:
      if poll == 0: 
        applicationStatus.set_text("Application ended successfully.\n")
        stopNext.set_label('Next')
      else: 
        applicationStatus.set_text("Application failed with  status: "+str(poll)+"\n")
        stopNext.set_label('Next')
  else: 
    applicationStatus.set_text("No application is running\n")
    stopNext.set_label('Stop')

  outputMessage.set_text('\n----Application Output----\n'+getOutputMessage()+'\n')

  _loop.set_alarm_in(1, refresh)

if __name__ == "__main__":
  # Global variables
  timeStatus = urwid.Text("Current time: "+datetime.datetime.now().strftime("%H:%M:%S")+'\n')
  totalErrorStatus = urwid.Text("Total validation errors: -1\n")
  usbErrorStatus = urwid.Text("Total USB errors: -1\n")
  applications = []
  applicationStatus = urwid.Text("No application is running\n")
  pid = os.getpid()
  stopNext = urwid.Button('Stop')
  currentDataFile = urwid.Text('data/counter.dat')
  outputMessage = urwid.Text('\n----Application Output----\n')
  usbStatus = urwid.Text("USB controller status (Should be 2): Unknown\n")
  promStatus = urwid.Text("Prom status: Unknown\n")

  main = urwid.Padding(menu(), left=2, right=2)

  top = urwid.Overlay(main, urwid.SolidFill(u'\N{MEDIUM SHADE}'),
      align='center', width=('relative', 80),
      valign='middle', height=('relative', 90),
      min_width=20, min_height=9)

  topFocus = top.get_focus_path()
  main_loop = urwid.MainLoop(top, palette=[('reversed', 'standout', '')])
  main_loop.set_alarm_in(1, refresh)
  main_loop.run()

