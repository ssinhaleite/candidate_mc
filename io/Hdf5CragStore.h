#ifndef CANDIDATE_MC_IO_HDF_CRAG_STORE_H__
#define CANDIDATE_MC_IO_HDF_CRAG_STORE_H__

#include <vigra/hdf5impex.hxx>
#include "Hdf5GraphReader.h"
#include "Hdf5GraphWriter.h"
#include "Hdf5DigraphReader.h"
#include "Hdf5DigraphWriter.h"
#include "Hdf5VolumeReader.h"
#include "Hdf5VolumeWriter.h"
#include "CragStore.h"

class Hdf5CragStore :
		public CragStore,
		public Hdf5GraphReader,
		public Hdf5GraphWriter,
		public Hdf5DigraphReader,
		public Hdf5DigraphWriter,
		public Hdf5VolumeReader,
		public Hdf5VolumeWriter {

public:

	Hdf5CragStore(std::string projectFile) :
		Hdf5GraphReader(_hdfFile),
		Hdf5GraphWriter(_hdfFile),
		Hdf5DigraphReader(_hdfFile),
		Hdf5DigraphWriter(_hdfFile),
		Hdf5VolumeReader(_hdfFile),
		Hdf5VolumeWriter(_hdfFile),
		_hdfFile(
				projectFile,
				vigra::HDF5File::OpenMode::ReadWrite) {}


	/**
	 * Store a candidate region adjacency graph (CRAG).
	 */
	void saveCrag(const Crag& crag) override;

	/**
	 * Retrieve the candidate region adjacency graph (CRAG) associated to this 
	 * store.
	 */
	void retrieveCrag(Crag& crag) override;

private:

	vigra::HDF5File _hdfFile;
};

#endif // CANDIDATE_MC_IO_HDF_CRAG_STORE_H__

