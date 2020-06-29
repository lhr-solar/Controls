import tkinter as tk
import csv
import os

def update_csv():
	""" Updates CAN.csv every 1 ms """

	"""Reading the Value"""

	currentValue = 0 # If exception is caught, skip writing by initializing to 0
	with open("Timer.csv", 'r') as csvfile:
		reader = csv.reader(csvfile, delimiter=',')
		
		try:
			for row in reader: # Should only be a single row
				currentValue = int(row[0])

		except IndexError: # If the csv is blank
			print("Blank CSV")


	if currentValue > 0: # Systick stops at 0
		with open("CAN.csv", 'w') as csvfile:
			writer = csv.writer(csvfile, delimiter=',')

			writer.writerow([currentValue - 1])

	window.after(1, update_csv)

if os.environ.get('DISPLAY','') == '':
    os.environ.__setitem__('DISPLAY', ':0')

window = tk.Tk()
window.after(1, update_csv)
window.mainloop()