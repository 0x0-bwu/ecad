import vtk
import os
def main():
    colors = vtk.vtkNamedColors()

    filename = os.path.dirname(__file__) + '/../test/data/simulation/thermal/hotmap.vtk'
    # filename = filename = os.path.dirname(__file__) + '/test.vtk'

    # Create the reader for the data.
    print('Loading ', filename)
    reader = vtk.vtkUnstructuredGridReader()
    reader.SetFileName(filename)
    reader.Update()

    # extractEdges = vtk.vtkExtractEdges()
    # extractEdges.SetInputConnection(reader.GetOutputPort())

    legendValues = vtk.vtkVariantArray()
    it = reader.GetOutput().NewCellIterator()
    it.InitTraversal()
    while not it.IsDoneWithTraversal():
        cell = vtk.vtkGenericCell()
        it.GetCell(cell)
        cellName = vtk.vtkCellTypes.GetClassNameFromTypeId(cell.GetCellType())
        # print(cellName, 'NumberOfPoints:', cell.GetNumberOfPoints(), 'CellDimension:', cell.GetCellDimension())
        legendValues.InsertNextValue(cellName)
        it.GoToNextCell()

    # The geometry
    geometryShrink = vtk.vtkShrinkFilter()
    geometryShrink.SetInputConnection(reader.GetOutputPort())
    geometryShrink.SetShrinkFactor(1.0)

    # NOTE: We must copy the originalLut because the CategoricalLegend
    # needs an indexed lookup table, but the geometryMapper uses a
    # non-index lookup table
    categoricalLut = vtk.vtkLookupTable()
    originalLut = reader.GetOutput().GetCellData().GetScalars().GetLookupTable()

    categoricalLut.DeepCopy(originalLut)
    categoricalLut.IndexedLookupOn()

    geometryMapper = vtk.vtkDataSetMapper()
    geometryMapper.SetInputConnection(geometryShrink.GetOutputPort())
    geometryMapper.SetScalarModeToUseCellData()
    geometryMapper.SetScalarRange(0, 11)

    geometryActor = vtk.vtkActor()
    geometryActor.SetMapper(geometryMapper)
    geometryActor.GetProperty().SetLineWidth(3)
    geometryActor.GetProperty().EdgeVisibilityOn()
    geometryActor.GetProperty().SetEdgeColor(0, 0, 0)

    contextView = vtk.vtkContextView()

    renderer = contextView.GetRenderer()

    renderWindow = contextView.GetRenderWindow()

    renderWindowInteractor = vtk.vtkRenderWindowInteractor()
    renderWindowInteractor.SetRenderWindow(renderWindow)

    renderer.AddActor(geometryActor)
    renderer.SetBackground(colors.GetColor3d('SlateGray'))

    aCamera = vtk.vtkCamera()
    aCamera.Azimuth(-40.0)
    aCamera.Elevation(50.0)

    renderer.SetActiveCamera(aCamera)
    renderer.ResetCamera()

    renderWindow.SetSize(640, 480)
    renderWindow.SetWindowName('ReadLegacyUnstructuredGrid')
    renderWindow.Render()

    renderWindowInteractor.Start()

if __name__ == '__main__':
    main()