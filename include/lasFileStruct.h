/**
	las �ļ��ṹ
**/
#ifndef _LAS_FILE_STRUCT_H_SPARCLE_2018_12_14
#define _LAS_FILE_STRUCT_H_SPARCLE_2018_12_14

#include <stdint.h>

namespace YupontLasFile {
		
#pragma  pack (push,1) 
	struct LasHeader
	{
		char m_signature[4];											//LASF		
		uint16_t m_sourceId;									//	
		uint16_t m_encoding;

		uint32_t  m_projectId1;									//			
		uint16_t m_projectId2;
		uint16_t m_projectId3;
		unsigned char m_projectId4[8];

		unsigned char m_versionMajor;									//��汾��		
		unsigned char m_versionMinor;									//С�汾��		

		char m_systemId[32];											//ϵͳ��		
		char m_softwareId[32];											//�����		
		uint16_t m_createDOY;										//������		
		uint16_t m_createYear;									//������	
		uint16_t m_headerSize;									//�ļ�ͷ��С	
		uint32_t m_dataOffset;										//����λ��
		uint32_t m_recordsCount;									//�䳤����VariableRecord����

		unsigned char m_pointDataFormatId;								//�������ݸ�ʽID Point Data Format ID
		unsigned short m_pointDataRecordLength;							//Point Data Record Length

		uint32_t m_pointRecordsCount;								//�������	
		uint32_t m_lasHeaderNumOfReturns[5];						//�ز�����
		
		double m_xScaleFactor;
		double m_yScaleFactor;
		double m_zScaleFactor;
		double m_xOffset;
		double m_yOffset;
		double m_zOffset;

		double m_maxX;
		double m_minX;
		double m_maxY;
		double m_minY;
		double m_maxZ;
		double m_minZ;
	};

	//--------------------------------------
	struct VariLenRecord
	{
		uint16_t reserved;
		char userId[16];
		unsigned short recordId;
		unsigned short recordLen;
		char description[32];
	};

	//--- GeoTiFF format -----------
	struct sKeyEntry
	{
		uint16_t wKeyID;
		uint16_t wTIFFTagLocation;
		uint16_t wCount;
		uint16_t wValue_Offset;
	};
	struct sGeoKeys         // LASF_Projection 34735
	{
		uint16_t wKeyDirectoryVersion;
		uint16_t wKeyRevision;
		uint16_t wMinorRevision;
		uint16_t wNumberOfKeys;
	};

	//--------------------------------------------

	struct RGB
	{
		uint16_t red;                //      2
		uint16_t green;              //      2
		uint16_t blue;               //      2   
	};

	struct Wave{
		unsigned char wavepacket;          //28      1
		uint64_t  offsetovaform; //29-36   8      1 
		uint32_t  waveform;           //37-40  4
		float returnpoint;                 //41-44   4   13
		float xt;                          //4
		float yt;                          //4
		float zt;                          //4
	};

	struct Pos
	{
		int32_t  X;                        //0-3         4                                      //0-3
		int32_t  Y;                        //4-7         4                                         //4-7
		int32_t  Z;                        //8-11        4      
	};

	struct PointFormat0
	{
		Pos m_position;				  //0-11
		uint16_t	m_intensity;      //12-13       2                           
		unsigned char  m_returnNumber : 3;          //14          1                               //14
		unsigned char  m_numOfReturns : 3;
		unsigned char  m_scanDir : 1;
		unsigned char  m_eageFlightLine : 1;
		unsigned char  m_classification;  //15          1         
		char m_scanAngle;                 //16          1  
		unsigned char m_userData;         //17         1  
		uint16_t m_PointSourceID;   //18 -19       2
	};

	struct PointFormat1
	{
		PointFormat0 m_p0;                       //0 -19       20
		double m_gpsTime;                  //20 -27      8   

	};

	struct PointFormat2
	{
		PointFormat0 m_p0;                        //0 -19       20
		RGB  m_rgb;                       //20-25       6
	};

	struct PointFormat3
	{
		PointFormat0 m_p0;                         //0 -19        20
		double m_gpstime;   ///              //20-27         8byte
		RGB m_rgb;                        //28-33         6
	};

	struct PointFormat4
	{
		PointFormat1 m_p1;                         //0-27    28
		Wave   m_wave;                      //29-56   29

	};
	struct PointFormat5
	{
		PointFormat3 m_p3;                         //0-33    34
		Wave  m_wave;                       //34-62    29

	};
	struct PointFormat6
	{
		Pos m_pos;
		uint16_t	m_intensity;       //12-13        2                                   
		unsigned char   m_other[2];        //14-15        2                            
		unsigned char  m_classification;   //16           1    
		unsigned char m_userData;          //17           1  
		short m_scanAngle;                 //18-19        2  
		uint16_t m_PointSourceID;    //20 -21       2
		double m_gpstime;   ///            //22-29        8
	};
	struct PointFormat7
	{
		PointFormat6 m_p6;                         //0-29       30
		RGB    m_rgb;                        //30-35       6
	};
	struct PointFormat8
	{
		PointFormat7 m_p7;                         //0-35      36
		unsigned short NIR;                //36-37     2

	};
	struct PointFormat9
	{
		PointFormat6 m_p6;                         //0-29     30
		Wave  m_wave;                       //30-58    29 

	};
	struct PointFormat10
	{
		PointFormat7 m_p7;                         //0-35       36
		Wave  m_wave;                       //36-62      29 
	};

#pragma pack(pop)	
	
}

#endif //_LAS_FILE_STRUCT_H_SPARCLE_2018_12_14