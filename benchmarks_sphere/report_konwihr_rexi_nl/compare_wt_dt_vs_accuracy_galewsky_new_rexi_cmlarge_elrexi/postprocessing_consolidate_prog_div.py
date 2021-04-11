#! /usr/bin/env python3

import sys
import math

from mule.plotting.Plotting import *
from mule.postprocessing.JobsData import *
from mule.postprocessing.JobsDataConsolidate import *

sys.path.append('../')
import pretty_plotting as pp
sys.path.pop()

#mule_plotting_usetex(False)

groups = ['runtime.timestepping_method', 'runtime.rexi_files_coefficients.0.unique_id_string']

tagnames_y = [
    'sphere_data_diff_prog_div.res_norm_l1',
    'sphere_data_diff_prog_div.res_norm_l2',
    'sphere_data_diff_prog_div.res_norm_linf',
]



j = JobsData('./job_bench_*', verbosity=0)

c = JobsDataConsolidate(j)
print("")
print("Groups:")
job_groups = c.create_groups(groups)
for key, g in job_groups.items():
    print(key)

for tagname_y in tagnames_y:

    params = []
    params += [
            {
                'tagname_x': 'runtime.timestep_size',
                'xlabel': "Timestep size (seconds)",
                'ylabel': pp.latex_pretty_names[tagname_y],
                'title': 'Timestep size vs. error',
                'xscale': 'log',
                'yscale': 'log',
            },
        ]

    params += [
            {
                'tagname_x': 'output.simulation_benchmark_timings.main_timestepping',
                'xlabel': "Wallclock time (seconds)",
                'ylabel': pp.latex_pretty_names[tagname_y],
                'title': 'Wallclock time vs. error',
                'xscale': 'log',
                'yscale': 'log',
            },
        ]


    for param in params:

        tagname_x = param['tagname_x']
        xlabel = param['xlabel']
        ylabel = param['ylabel']
        title = param['title']
        xscale = param['xscale']
        yscale = param['yscale']

        print("*"*80)
        print("Processing tag "+tagname_x)
        print("*"*80)



        if True:
            """
            Plotting format
            """

            # Filter out errors beyond this value!
            def data_filter(x, y, jobdata):
                if y == None:
                    return True

                x = float(x)
                y = float(y)

                if math.isnan(y):
                    return True

                if 'prog_phi' in tagname_y:
                    return False
                    if 'l1' in tagname_y:
                        if y > 1e5:
                            print("Sorting out L1 data "+str(y))
                            return True
                    elif 'l2' in tagname_y:
                        if y > 1e5:
                            print("Sorting out L2 data "+str(y))
                            return True
                    elif 'linf' in tagname_y:
                        if y > 1e6:
                            print("Sorting out Linf data "+str(y))
                            return True
                    else:
                        raise Exception("Unknown y tag "+tagname_y)

                elif 'prog_div' in tagname_y:
                    if 'l1' in tagname_y:
                        if y > 1e1:
                            print("Sorting out L1 data "+str(y))
                            return True
                    elif 'l2' in tagname_y:
                        if y > 1e1:
                            print("Sorting out L2 data "+str(y))
                            return True
                    elif 'linf' in tagname_y:
                        if y > 1e2:
                            print("Sorting out Linf data "+str(y))
                            return True
                    else:
                        raise Exception("Unknown y tag "+tagname_y)

                else:
                    print("TODO")

                return False



            d = JobsData_GroupsPlottingScattered(
                    job_groups,
                    tagname_x,
                    tagname_y,
                    data_filter = data_filter
                )

            fileid = "output_plotting_"+tagname_x.replace('.', '-').replace('_', '-')+"_vs_"+tagname_y.replace('.', '-').replace('_', '-')


            if True:
                #
                # Proper naming and sorting of each label
                #

                # new data dictionary
                data_new = {}
                for key, data in d.data.items():

                    print("Processing "+key)
                    print(" + x_values: ", str(data['x_values']))
                    print(" + y_values: ", str(data['y_values']))

                    if len(data['x_values']) == 0:
                        print(" + skipping "+key+": No value available")
                        continue

                    if '_exp_' in key:
                        if len(data['x_values']) <= 1:
                            print(" + skipping "+key+": Only one value available")
                            continue


                    s = key.split('__')

                    # generate nice tex label
                    if len(s) == 1:
                        data['label'] = pp.get_pretty_name(s[0])
                    else:
                        data['label'] = pp.get_pretty_name(s[0])+" "+s[1].replace("_", " ")

                    key_new = pp.get_pretty_name_order(s[0])+'_'+key

                    # copy data
                    data_new[key_new] = copy.copy(data)

                # Copy back new data table
                d.data = data_new

            p = Plotting_ScatteredData()


            def fun(p):
                from matplotlib import ticker
                from matplotlib.ticker import FormatStrFormatter

                plt.tick_params(axis='x', which='minor')
                p.ax.xaxis.set_minor_formatter(FormatStrFormatter("%.0f"))
                p.ax.xaxis.set_major_formatter(FormatStrFormatter("%.0f"))

                p.ax.xaxis.set_minor_locator(ticker.LogLocator(subs=[1.5, 2.0, 3.0, 5.0]))

                for tick in p.ax.xaxis.get_minor_ticks():
                    tick.label.set_fontsize(8) 


                plt.tick_params(axis='y', which='minor')
                p.ax.yaxis.set_minor_formatter(FormatStrFormatter("%.1e"))
                p.ax.yaxis.set_major_formatter(FormatStrFormatter("%.1e"))
 
                p.ax.yaxis.set_minor_locator(ticker.LogLocator(subs=[1.5, 2.0, 3.0, 5.0]))

                for tick in p.ax.yaxis.get_minor_ticks():
                    tick.label.set_fontsize(6) 



            for key, data in d.get_data_float().items():
                print(key, data)

            annotate_text_template = "{:.1f} / {:.3f}"
            p.plot(
                    data_plotting = d.get_data_float(),
                    xlabel = xlabel,
                    ylabel = ylabel,
                    title = title,
                    xscale = xscale,
                    yscale = yscale,
                    #annotate = True,
                    #annotate_each_nth_value = 3,
                    #annotate_fontsize = 6,
                    #annotate_text_template = annotate_text_template,
                    legend_fontsize = 6,
                    grid = True,
                    outfile = fileid+".pdf",
                    lambda_fun = fun,
                )

            print("Data plotting:")
            d.print()
            d.write(fileid+".csv")

        print("Info:")
        print("    NaN: Errors in simulations")
        print("    None: No data available")
