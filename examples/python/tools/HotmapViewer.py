import vtk
import sys
import os
def view_hotmap(filename):
    if not os.path.exists(filename) :
        print(f'Error: {filename} not exists')
        return

    colors = vtk.vtkNamedColors()

    # Create the reader for the data.
    print('Loading ', filename)
    reader = vtk.vtkUnstructuredGridReader()
    reader.SetFileName(filename)
    reader.Update()

    # The geometry
    geometry_shrink = vtk.vtkShrinkFilter()
    geometry_shrink.SetInputConnection(reader.GetOutputPort())
    geometry_shrink.SetShrinkFactor(1.0)

    scalar = reader.GetOutput().GetCellData().GetScalars()
    scalar_lut = scalar.GetLookupTable()
    scalar_bar = vtk.vtkScalarBarActor()
    scalar_bar.SetLookupTable(scalar_lut)  # Link to the mapper's LUT
    scalar_bar.SetTitle("Temperature")
    scalar_bar.SetHeight(0.5)  # Adjust height as needed (0-1)
    scalar_bar.SetWidth(0.05)  # Adjust width as needed (0-1)

    title_prop = scalar_bar.GetTitleTextProperty()
    title_prop.SetFontSize(3)
    title_prop.SetColor(colors.GetColor3d('Black'))

    label_prop = scalar_bar.GetLabelTextProperty()
    label_prop.SetColor(colors.GetColor3d('Black'))
    label_prop.SetFontSize(3)

    scalar_range = scalar.GetRange()
    geometry_mapper = vtk.vtkDataSetMapper()
    geometry_mapper.SetInputConnection(geometry_shrink.GetOutputPort())
    geometry_mapper.SetScalarModeToUseCellData()
    geometry_mapper.SetScalarRange(scalar_range[0], scalar_range[1])

    geometry_actor = vtk.vtkActor()
    geometry_actor.SetMapper(geometry_mapper)
    geometry_actor.GetProperty().SetLineWidth(0.01)
    geometry_actor.GetProperty().EdgeVisibilityOn()
    geometry_actor.GetProperty().SetEdgeColor(0, 0, 0)
    geometry_actor.GetProperty().SetEdgeOpacity(0.5)
    geometry_actor.SetScale(1.0, 1.0, 1.0)

    context_view = vtk.vtkContextView()

    renderer = context_view.GetRenderer()

    render_window = context_view.GetRenderWindow()

    render_window_interactor = vtk.vtkRenderWindowInteractor()
    render_window_interactor.SetRenderWindow(render_window)

    renderer.AddActor(geometry_actor)
    renderer.SetBackground(colors.GetColor3d('White'))
    # renderer.SetBackground(colors.GetColor3d('SlateGray'))
    renderer.AddActor(scalar_bar)

    camera = vtk.vtkCamera()
    camera.Azimuth(0)
    camera.Elevation(0)

    renderer.SetActiveCamera(camera)
    renderer.ResetCamera()

    render_window.SetSize(3000, 2000)
    render_window.SetWindowName('ReadLegacyUnstructuredGrid')
    render_window.Render()

    render_window_interactor.Start()

def main() :
    if len(sys.argv) < 2 :
        print('Error: please specify vtk file')
        return

    filename = sys.argv[1]    
    view_hotmap(filename)

if __name__ == '__main__' :
    main()
