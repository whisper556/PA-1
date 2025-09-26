import matplotlib.pyplot as plt

# File containing the results
RESULTS_FILE = "transfer_times.txt"

file_sizes = []
times = []

# Read the results file
with open(RESULTS_FILE, "r") as f:
    next(f)  # skip header
    for line in f:
        parts = line.strip().split()
        if len(parts) >= 3:
            size_str = parts[0]
            time_sec = float(parts[1])
            
            # Convert size to MB for plotting
            if size_str.endswith("M"):
                size_mb = float(size_str[:-1])
            elif size_str.endswith("K"):
                size_mb = float(size_str[:-1]) / 1024
            else:
                size_mb = float(size_str)
            
            file_sizes.append(size_mb)
            times.append(time_sec)

# Plot
plt.figure(figsize=(8,5))
plt.plot(file_sizes, times, marker='o', linestyle='-', color='blue')
plt.xlabel("File Size (MB)")
plt.ylabel("Transfer Time (s)")
plt.title("PA1 File Transfer Time vs File Size")
plt.grid(True)

# Annotate each point with its time
for i, t in enumerate(times):
    plt.text(file_sizes[i], t, f"{t:.2f}s", fontsize=9, ha='right', va='bottom')

plt.tight_layout()
plt.savefig("transfer_times_chart.png")  # save as PNG
print("Chart saved as transfer_times_chart.png")

