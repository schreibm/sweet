#! /usr/bin/env python3
#
# Author: Raphael Schilling <raphael.schilling@tum.de>
#

from mule_local.rexi.pcirexi.PCIREXI import PCIREXI
from mule_local.rexi.pcirexi.contour.RectangleContour import RectangleContour
from mule_local.rexi.pcirexi.section.InterpolationSettings import InterpolationSettings
from mule_local.rexi.pcirexi.section.QuadratureSettings import QuadratureSettings


class LRREXI:
    def setup(self, width:float, height:float, center:complex, N:int):
        pcirexi = PCIREXI()
        contour = RectangleContour(width, height, center)
        i_s = InterpolationSettings(4, 2, 'equidistant', False, False)
        q_s = QuadratureSettings(overall_quadrature_points=N,
                                 quadrature_method="legendre",
                                 distribute_quad_points_based_on_arc_length=True)
        coeffs = pcirexi.setup_pcirexi(contour=contour,
                                            interpolation_settings=i_s,
                                            quadrature_settings=q_s)

        unique_id_string = "LRREXI_"
        unique_id_string += "phi0" #self.function_name
        unique_id_string += "_N" + str(N)
        unique_id_string += "_w" + str(width)
        unique_id_string += "_h" + str(height)
        unique_id_string += "_c" + str(center)

        coeffs.unique_id_string = unique_id_string
        return coeffs
