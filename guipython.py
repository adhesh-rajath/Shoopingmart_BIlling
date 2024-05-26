import tkinter as tk
from tkinter import ttk
import serial
import threading

# Set up the serial port
ser = serial.Serial('COM8', 9600)

# Initialize total rate
total_rate = 0
check=0

def read_from_arduino():
    global total_rate, check

    while True:
        if ser.inWaiting():
            # Read a line from the serial port
            line = ser.readline().decode('utf-8').strip()
            print(line)
            # If the line starts with "Item: ", update the item label
            if line.startswith('Item: '):
                item = line.split(': ')[1]
                item_label.config(text='Item: ' + item)

            # If the line starts with "Rate: ", update the rate label and add to billing table
            elif line.startswith('Rate: '):
                rate = line.split(': ')[1]
                rate_label.config(text='Rate: ' + rate)
                billing_table.insert('', 'end', values=(item, rate))
                total_rate += float(rate)
                total_label.config(text='Total: ' + str(total_rate))

            elif line=='close':
                check=0
                switch_frame()

            elif line == 'open':
                check=1
                switch_frame()

def close_and_send_signal():
    global total_rate, check

    # Clear the table
    for i in billing_table.get_children():
        billing_table.delete(i)

    # Reset the labels
    item_label.config(text='Item: ')
    rate_label.config(text='Rate: ')
    total_label.config(text='Total: 0')

    # Reset the total rate
    total_rate = 0

    # Set check to 0 and switch frame
    check = 0
    switch_frame()
    ser.write(b'button clicked\n')
    
def switch_frame():
    if check == 0:
        table_frame.pack_forget()
        image_frame.pack()
    else:
        image_frame.pack_forget()
        table_frame.pack()

global root, item_label, rate_label, total_label, billing_table, image_frame, table_frame

root = tk.Tk()

# Create the image frame
image_frame = tk.Frame(root)
image = tk.PhotoImage(file="C://Users//aadiu//Pictures//favicons.png")  # replace with your image file
image_label = tk.Label(image_frame, image=image)
image_label.pack()

# Create the table frame
table_frame = tk.Frame(root)
item_label = tk.Label(table_frame, text='Item: ')
item_label.pack()
rate_label = tk.Label(table_frame, text='Rate: ')
rate_label.pack()
total_label = tk.Label(table_frame, text='Total: 0')
total_label.pack()

# Set up the billing table
billing_table = ttk.Treeview(table_frame, columns=('Item', 'Rate'), show='headings')
billing_table.heading('Item', text='Item')
billing_table.heading('Rate', text='Rate')
billing_table.pack()

# Add a button that sends a signal to the Arduino and closes the window when clicked
close_button = tk.Button(table_frame, text='Close', command=close_and_send_signal)
close_button.pack()

# Start reading from the Arduino
threading.Thread(target=read_from_arduino, daemon=True).start()
print(check)
# Start the GUI event loop
root.mainloop()