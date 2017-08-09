import matplotlib.pyplot as plt
import os

#prepare the filenames to read
dimension = int(input("Enter the dimension of the tensor [required for file parsing]: "))
filenames = list()
for i in range(dimension):
    for j in range(dimension + 1)[i + 1:]:
        filenames.append('mode_' + str(i) + '_' + str(j) + '.metric')

def plot_data_in_folder(folder, output_fname):
    for filename in filenames:
        fo = open('./' + folder + '/' + filename)
        
        distance_metrics = []
        normalized_pair_metrics = []
        pair_metrics = []

        line = fo.readline()
        while (line != ""):

            distance_metric, normalized_pairwise, pairwise = map(float, line.split())

            distance_metrics.append(distance_metric)
            normalized_pair_metrics.append(normalized_pairwise)
            pair_metrics.append(pairwise)

            line = fo.readline()

            
        f, (dist_plot, npair_plot, pair_plot) = plt.subplots(3, sharex = True)
        # values gathered, prepare the graphs
        # 1 - Plot the distance metric graph
        
        dist_plot.set_title(filename + ' distance')
        dist_plot.plot(distance_metrics, 'green')
        
        # 2 - Plot the normalized pairwise metric
        npair_plot.set_title(filename + ' norm pairwise')
        npair_plot.plot(normalized_pair_metrics, 'red')
        
        # 3 - Plot the pairwise metrics
        pair_plot.set_title(filename + ' pairwise')
        pair_plot.plot(pair_metrics, 'orange')
        
        f.subplots_adjust(hspace=1)
        plt.setp([a.get_xticklabels() for a in f.axes[:-1]], visible=False)
        if not os.path.exists(output_fname):
            os.makedirs(output_fname)
        plt.savefig('./' + output_fname + '/' + filename + ".png")
        plt.close()
        
        print ('Plotted ' + filename)

plot_data_in_folder('ordered_metrics', 'ordered_metric_images')
plot_data_in_folder('ordered_metrics', 'natural_metric_images')