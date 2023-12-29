import os
import vtk

def main():
    filename = os.path.dirname(__file__) + '/../test/data/simulation/thermal/mesh.vtk'
    # filename = os.path.dirname(__file__) + '/test.vtk'

    # Create the reader for the data.
    reader = vtk.vtkUnstructuredGridReader()
    reader.SetFileName(filename)
    reader.Update()

    # Set mapper and renderer
    # mapper = vtk.vtkUnstructuredGridMapper()
    # mapper.SetInputConnection(reader.GetOutputPort())
    # mapper.SetLookupTable(lut)
    # mapper.SetColorModeToMapScalars()

    bounds = reader.GetOutput().GetBounds()
    center = reader.GetOutput().GetCenter()
    
    colors = vtk.vtkNamedColors()
    renderer = vtk.vtkRenderer()
    renderer.SetBackground(colors.GetColor3d('Wheat'))
    renderer.UseHiddenLineRemovalOn()
    # renderer.AddActor(mapper)

    renderWindow = vtk.vtkRenderWindow()
    renderWindow.AddRenderer(renderer)
    renderWindow.SetSize(640, 480)

    interactor = vtk.vtkRenderWindowInteractor()
    interactor.SetRenderWindow(renderWindow)
    
    # shrink = vtkShrinkFilter()
    # shrink.SetInputConnection(reader.GetOutputPort())
    # shrink.SetShrinkFactor(0.6)

    dataSetMapper = vtk.vtkDataSetMapper()
    dataSetMapper.SetInputConnection(reader.GetOutputPort())
    dataSetMapper.ScalarVisibilityOff()

    actor = vtk.vtkActor()
    actor.SetMapper(dataSetMapper)
    # actor.GetProperty().SetDiffuseColor(colors.GetColor3d('tomato'))
    actor.GetProperty().EdgeVisibilityOn()
    actor.GetProperty().SetLineWidth(0.2)
    actor.SetScale(1.0, 1.0, 3)

    trans = vtk.vtkTransform()
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