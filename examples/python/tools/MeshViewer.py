import os
import sys
import vtk

def view_mesh(filename):

    reader = vtk.vtkUnstructuredGridReader()
    reader.SetFileName(filename)
    reader.Update()

    bounds = reader.GetOutput().GetBounds()
    center = reader.GetOutput().GetCenter()
    
    colors = vtk.vtkNamedColors()
    renderer = vtk.vtkRenderer()
    renderer.SetBackground(colors.GetColor3d('White'))
    renderer.UseHiddenLineRemovalOn()
    # renderer.AddActor(mapper)

    render_window = vtk.vtkRenderWindow()
    render_window.AddRenderer(renderer)
    render_window.SetSize(640, 480)

    interactor = vtk.vtkRenderWindowInteractor()
    interactor.SetRenderWindow(render_window)
    
    # shrink = vtk.vtkShrinkFilter()
    # shrink.SetInputConnection(reader.GetOutputPort())
    # shrink.SetShrinkFactor(0.6)

    dataSetMapper = vtk.vtkDataSetMapper()
    dataSetMapper.SetInputConnection(reader.GetOutputPort())
    dataSetMapper.ScalarVisibilityOff()

    actor = vtk.vtkActor()
    actor.SetMapper(dataSetMapper)
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
    render_window.Render()
    render_window.SetWindowName('mesh viewer')
    render_window.Render()

    interactor.Start()

def main() :
    if len(sys.argv) < 2 :
        print('Error: please specify vtk file')
        return

    filename = sys.argv[1]    
    view_mesh(filename)

if __name__ == '__main__' :
    main()