import os
import matplotlib.pyplot as plt

def load_data(filename) :
    results = []
    with open(filename) as f :
        for line in f :
            args = line.strip().split(',')[:-1]
            if 0 == len(results) :
                for i in range(len(args)) :
                    results.append([])
            for i in range(len(args)) :
                results[i].append(float(args[i]))
    return results

def main():
    filename = os.path.dirname(__file__) + '/../test/data/simulation/thermal/trans.txt'
    results = load_data(filename)
    for i in range(1, len(results)) :
        plt.plot(results[0], results[i], label = f'prob_{i}')
    plt.legend()
    plt.show()

if __name__ == '__main__':
    main()