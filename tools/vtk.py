import collections
import os

# noinspection PyUnresolvedReferences
import vtkmodules.vtkInteractionStyle
# noinspection PyUnresolvedReferences
import vtkmodules.vtkRenderingOpenGL2
from vtkmodules.vtkCommonColor import vtkNamedColors
from vtkmodules.vtkCommonDataModel import (
    vtkCellTypes,
    vtkPlane
)
from vtkmodules.vtkCommonTransforms import vtkTransform
from vtkmodules.vtkFiltersGeneral import vtkShrinkFilter
from vtkmodules.vtkIOLegacy import vtkUnstructuredGridReader
from vtkmodules.vtkRenderingCore import (
    vtkActor,
    vtkDataSetMapper,
    vtkRenderWindow,
    vtkRenderWindowInteractor,
    vtkRenderer
)

def main():
    filename = '/home/bwu/code/myRepo/ecad/test/data/simulation/thermal/prisma.vtk'

    # Create the reader for the data.
    reader = vtkUnstructuredGridReader()
    reader.SetFileName(filename)
    reader.Update()

    bounds = reader.GetOutput().GetBounds()
    center = reader.GetOutput().GetCenter()
    
    colors = vtkNamedColors()
    renderer = vtkRenderer()
    renderer.SetBackground(colors.GetColor3d('Wheat'))
    renderer.UseHiddenLineRemovalOn()

    renderWindow = vtkRenderWindow()
    renderWindow.AddRenderer(renderer)
    renderWindow.SetSize(640, 480)

    interactor = vtkRenderWindowInteractor()
    interactor.SetRenderWindow(renderWindow)
    
    shrink = vtkShrinkFilter()
    shrink.SetInputConnection(reader.GetOutputPort())
    shrink.SetShrinkFactor(0.6)
        
    dataSetMapper = vtkDataSetMapper()
    dataSetMapper.SetInputConnection(shrink.GetOutputPort())
    dataSetMapper.ScalarVisibilityOff()

    actor = vtkActor()
    actor.SetMapper(dataSetMapper)
    actor.GetProperty().SetDiffuseColor(colors.GetColor3d('tomato'))
    actor.GetProperty().EdgeVisibilityOn()
    actor.GetProperty().SetLineWidth(0.2)
    # actor.SetScale(1.0, 1.0, 50)

    trans = vtkTransform()
    trans.Translate(-center[0], -center[1], -center[2])
    actor.SetUserTransform(trans)

    renderer.AddViewProp(actor)

    renderer.ResetCamera()
    renderer.GetActiveCamera().Dolly(1.4)
    renderer.ResetCameraClippingRange()
    renderWindow.Render()
    renderWindow.SetWindowName('mesh viewer')
    renderWindow.Render()

    interactor.Start()

if __name__ == '__main__':
    main()