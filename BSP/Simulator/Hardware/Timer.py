import csv
import os
import fcntl

# Path of file
file = "BSP/Simulator/Hardware/Data/Timer.csv"

def update():
	""" Decrements the current value of the two timer simulations in Timer.csv every 1 ms """

	# Create the file if it doesn't exist yet
	os.makedirs(os.path.dirname(file), exist_ok=True)
	if not os.path.exists(file):
		open(file, 'w')
		file.close()

	current_values = []
	reload_values = []

	with open(file, 'r') as csvfile:
		fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)
		reader = csv.reader(csvfile, delimiter=',')
		
		for row in reader: # Should be 2 rows
			if row is not []:
				current_values.append(int(row[0]))
				reload_values.append(int(row[1]))
		fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)

	with open(file, 'w') as csvfile:
		fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)
		writer = csv.writer(csvfile, delimiter=',')

		for timer in range(0,2):

			try:
				if current_values[timer] > 0: # If timer is still running
					writer.writerow([current_values[timer] - 1, reload_values[timer]])
				else: # When timer hits 0, reload
					writer.writerow([reload_values[timer], reload_values[timer]])

			except IndexError: # If a row is blank
				pass
		fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)
