import tkinter as tk
from tkinter import ttk
import serial
from serial.tools import list_ports
import time


class PointsTable:

    def __init__(self, master):
        # MASTER
        self.master = master
        # WIDGETS:
        # Params
        entry_width = 7
        button_width = 5
        col_width = 100

        # Entries
        # Frame contains inputs for position, time and wash cycles
        self.entryFrame = ttk.Frame(self.master, width=4 * col_width)

        self.processLabel = tk.Label(self.entryFrame, text='Process:')
        self.pLabel = tk.Label(self.entryFrame, text='Position:')
        self.tLabel = tk.Label(self.entryFrame, text='Time:')
        self.wLabel = tk.Label(self.entryFrame, text='Wash cycles:')

        self.processLabel.grid(row=0, column=0)
        self.pLabel.grid(row=0, column=1)
        self.tLabel.grid(row=0, column=2)
        self.wLabel.grid(row=0, column=3)

        self.processEntry = tk.Entry(self.entryFrame)
        self.processEntry.config(width=entry_width)
        self.processEntry.grid(row=1, column=0)

        vcmd = master.register(self.validate)

        self.pEntry = tk.Entry(self.entryFrame, validate='key', validatecommand=(vcmd, '%P'))
        self.pEntry.config(width=entry_width)
        self.pEntry.grid(row=1, column=1)

        self.tEntry = tk.Entry(self.entryFrame, validate='key', validatecommand=(vcmd, '%P'))
        self.tEntry.config(width=entry_width)
        self.tEntry.grid(row=1, column=2)

        self.wEntry = tk.Entry(self.entryFrame, validate='key', validatecommand=(vcmd, '%P'))
        self.wEntry.config(width=entry_width)
        self.wEntry.grid(row=1, column=3)

        # Frame on top of the grid for layer # and any extra features
        self.extraFrame = tk.Frame(self.master)
        self.lLabel = tk.Label(self.extraFrame, text="Layers:")
        self.lLabel.grid(row=0, column=0, sticky=tk.W)
        self.lEntry = tk.Entry(self.extraFrame, validate='key', validatecommand=(vcmd, '%P'))
        self.lEntry.config(width=4)
        self.lEntry.grid(row=0, column=1, sticky=tk.W)

        # Buttons
        self.buttonFrame = ttk.Frame(self.master)

        self.enter = tk.Button(self.buttonFrame, text='Enter', command=self.enter_point)
        self.enter.config(width=button_width)
        self.enter.grid(row=0, column=0)

        self.delete = tk.Button(self.buttonFrame, text='Delete', command=self.delete_point)
        self.delete.config(state='disabled')
        self.delete.config(width=button_width)
        self.delete.grid(row=0, column=1)

        self.edit = tk.Button(self.buttonFrame, text='Edit', command=self.edit_point)
        self.edit.config(state='disabled')
        self.edit.config(width=button_width)
        self.edit.grid(row=0, column=2)

        # Treeview
        columns = ['position', 'time', 'wash']
        self.infoTree = ttk.Treeview(master, columns=columns)

        self.infoTree.heading('#0', text="Process")
        self.infoTree.heading('position', text='Position (mm)')
        self.infoTree.heading('time', text='Time (minutes)')
        self.infoTree.heading('wash', text='Wash cycles')

        self.infoTree.column('#0', width=col_width)
        for index in columns:
            self.infoTree.column(index, width=col_width)

        self.infoTree.bind('<<TreeviewSelect>>', self.click_on_point)

        # LAYOUT
        self.extraFrame.grid(row=0, column=0, sticky=tk.W)
        self.infoTree.grid(row=1, column=0)
        self.entryFrame.grid(row=2, column=0)
        self.buttonFrame.grid(row=3, column=0)

    def validate(self, new_text):
        if not new_text:  # the field is being cleared
            return True
        try:
            float(new_text)
            return True
        except ValueError:
            return False

    def clear_entries(self):
        self.processEntry.delete(0, tk.END)
        self.pEntry.delete(0, tk.END)
        self.tEntry.delete(0, tk.END)
        self.wEntry.delete(0, tk.END)

    def enter_point(self):
        p_val = self.pEntry.get()
        t_val = self.tEntry.get()
        w_val = self.wEntry.get()

        if w_val == '':
            w_val = 0

        if p_val == '' or t_val == '':
            print("error")
        else:
            process_name = self.processEntry.get()

            while self.infoTree.exists(process_name):
                process_name = self.unique_namer(process_name)

            self.infoTree.insert('',
                                 tk.END,
                                 iid=process_name,
                                 text=process_name,
                                 values=[p_val, t_val, w_val])
            self.clear_entries()

    def click_on_point(self, event):
        # Enables buttons to be pressed:
        point_name = self.infoTree.focus()

        if point_name != '':
            pointvals = self.infoTree.set(point_name)

            self.clear_entries()

            self.processEntry.insert(0, point_name)
            self.pEntry.insert(0, pointvals['position'])
            self.tEntry.insert(0, pointvals['time'])
            self.wEntry.insert(0, pointvals['wash'])

            self.delete.config(state='normal')
            self.edit.config(state='normal')

            print(self.infoTree.focus())

    def edit_point(self):
        point_name = self.infoTree.focus()

        self.edit.config(state='disabled')
        self.delete.config(state='disabled')

        self.infoTree.set(point_name, 'position', self.pEntry.get())
        self.infoTree.set(point_name, 'time', self.tEntry.get())
        self.infoTree.set(point_name, 'wash', self.wEntry.get())

        self.infoTree.selection_remove(point_name)
        self.clear_entries()

        print(f'{point_name} edited')

    def delete_point(self):
        point_name = self.infoTree.focus()

        self.edit.config(state='disabled')
        self.delete.config(state='disabled')

        self.infoTree.delete(point_name)
        self.clear_entries()

        print(f'{point_name} deleted')

    @property
    def get_array(self):
        tempiid = self.infoTree.get_children()
        points_array = [f"<L {self.lEntry.get()}>"]

        for key in tempiid:
            p_temp = self.infoTree.set(key, 'position')
            p_temp = str(round(50 * float(p_temp)))
            t_temp = self.infoTree.set(key, 'time')
            t_temp = str(round(60 * float(t_temp)))
            w_temp = self.infoTree.set(key, 'wash')

            tempstring = '<P ' + p_temp + ', ' + t_temp + ', ' + w_temp + '>'

            points_array.append(tempstring)

        points_array.append('<S>')

        return points_array

    def unique_namer(self, title):
        components = title.split()

        if components[-1].isnumeric():
            new_num = int(components[-1]) + 1
            components[-1] = str(new_num)
        else:
            components.append(' 1')

        return ' '.join(components)


class SerialWidget:

    def __init__(self, master):
        self.master = master

        # Value is True once the arduino is connected
        self.commsOn = False
        self.arduino = serial.Serial()

        # WIDGETS:

        # List box of all the serial connections
        self.ports = list_ports.comports()
        self.devices = [p.device for p in self.ports]
        self.deviceList = tk.Listbox(master)
        for p in self.devices:
            self.deviceList.insert(tk.END, p)
        self.deviceList.bind('<<ListboxSelect>>', self.enable_select)

        # Entry to input into the monitor
        self.serialEntry = tk.Entry(master)

        # Textbox to display serial data
        self.monitor = tk.Text(master)
        self.monitor['state'] = 'disabled'

        # Buttons
        self.connectButton = tk.Button(master, text='CONNECT', state='disabled', command=self.connect)
        self.sendButton = tk.Button(master, text='SEND', command=self.send)

        # Layout
        self.deviceList.grid(row=0, column=0)
        self.connectButton.grid(row=1, column=0)

        self.monitor.grid(row=0, column=1, rowspan=4)
        self.serialEntry.grid(row=2, column=0)
        self.sendButton.grid(row=3, column=0)

    def send(self):
        self.monitor['state'] = 'normal'

        temptext = self.serialEntry.get()
        self.serialEntry.delete(0, tk.END)

        self.monitor.insert(tk.END, temptext + '\n')

        if self.commsOn:
            self.arduino.write(bytes(temptext, 'utf-8'))
            time.sleep(0.05)

        self.monitor['state'] = 'disabled'

    def public_send(self, text):
        self.monitor['state'] = 'normal'

        self.monitor.insert(tk.END, text + '\n')

        if self.commsOn:
            self.arduino.write(bytes(text, 'utf-8'))
            time.sleep(0.05)

    def enable_select(self, event):
        self.connectButton.config(state='normal')

    def connect(self):
        index = self.deviceList.curselection()
        port_name = self.deviceList.get(index)

        self.arduino.baudrate = 9600
        self.arduino.port = port_name
        self.arduino.bytesize = serial.EIGHTBITS
        self.arduino.parity = serial.PARITY_NONE
        self.arduino.timeout = .1

        self.arduino.open()

        if self.arduino.isOpen():
            self.commsOn = True
            self.always_read()

        print('Connected to ' + port_name)

    def always_read(self):
        temptext_prev = ""
        if self.commsOn and self.arduino.in_waiting:
            temptext = self.arduino.readline()

            if temptext != temptext_prev:
                self.monitor['state'] = 'normal'
                self.monitor.insert(tk.END, temptext.decode('utf-8'))
                self.monitor['state'] = 'disabled'

            temptext_prev = temptext

        self.master.after(500, self.always_read)


def submit_to_terminal():
    print("why")
    procedure_array = myPoints.get_array
    print(mySerial.commsOn)
    if mySerial.commsOn:
        for item in procedure_array:
            mySerial.public_send(item)


root = tk.Tk()

points_frame = tk.LabelFrame(root, text='Points:', padx=10, pady=10)
serial_frame = tk.LabelFrame(root, text='Serial:', padx=10, pady=10)

myPoints = PointsTable(points_frame)
mySerial = SerialWidget(serial_frame)

points_frame.grid(row=0, column=0, sticky=tk.NW)
serial_frame.grid(row=0, column=1, sticky=tk.NE)

print_button = tk.Button(root, text='SUBMIT', command=submit_to_terminal)
print_button.grid(row=1, column=0, rowspan=2)

root.mainloop()
