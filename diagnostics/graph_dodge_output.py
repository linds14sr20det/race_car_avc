import ADC0832
import time
import math
import matplotlib.pyplot as plt
import matplotlib.animation as animation

def init():
    print("Recording sensor data:")
    ADC0832.setup()

def animate(i, ys):
    analogVal = ADC0832.getResult()
    Vr = 5 * float(analogVal) / 255
    data.append(Vr)
    ys.append(Vr)
    ys = ys[-x_len:]
    line.set_ydata(ys)
    f.write(str(Vr))
    f.write("\n")
    return line,

if __name__ == '__main__':
    init()
    try:
        f = open("vibdata.csv", "a+")
        f.truncate(0)
        data = []
        # Parameters
        x_len = 200         # Number of points to display
        y_range = [0, 5]  # Range of possible Y values to display

        # Create figure for plotting
        fig = plt.figure()
        ax = fig.add_subplot(1, 1, 1)
        xs = list(range(0, 200))
        ys = [0] * x_len
        ax.set_ylim(y_range)

        line, = ax.plot(xs, ys)
        
        ani = animation.FuncAnimation(fig, animate, fargs=(ys,), interval=50, blit=True)
        ani.save('animation.mp4')
        plt.show()
    except KeyboardInterrupt:
        f.close()
        ADC0832.destroy()
    print('The end!')
