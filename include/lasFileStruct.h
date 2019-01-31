/**
	las �ļ��ṹ
**/
#ifndef _LAS_FILE_STRUCT_H_SPARCLE_2018_12_14
#define _LAS_FILE_STRUCT_H_SPARCLE_2018_12_14

namespace YupontLasFile {
		
#pragma  pack (push,1) 
	struct LasHeader
	{
		char m_signature[4];											//LASF		
		unsigned short m_sourceId;									//	
		unsigned short m_encoding;

		unsigned long  m_projectId1;									//			
		unsigned short m_projectId2;
		unsigned short m_projectId3;
		unsigned char m_projectId4[8];

		unsigned char m_versionMajor;									//��汾��		
		unsigned char m_versionMinor;									//С�汾��		

		char m_systemId[32];											//ϵͳ��		
		char m_softwareId[32];											//�����		
		unsigned short m_createDOY;										//������		
		unsigned short m_createYear;									//������	
		unsigned short m_headerSize;									//�ļ�ͷ��С	
		unsigned long m_dataOffset;										//����λ��
		unsigned long m_recordsCount;									//�䳤����VariableRecord����

		unsigned char m_pointDataFormatId;								//�������ݸ�ʽID Point Data Format ID
		unsigned short m_pointDataRecordLength;							//Point Data Record Length

		unsigned long m_pointRecordsCount;								//�������	
		unsigned long m_lasHeaderNumOfReturns[5];						//�ز�����
		
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
		unsigned short reserved;
		char userId[16];
		unsigned short recordId;
		unsigned short recordLen;
		char description[32];
	};

	//--- GeoTiFF format -----------
	struct sKeyEntry
	{
		unsigned short wKeyID;
		unsigned short wTIFFTagLocation;
		unsigned short wCount;
		unsigned short wValue_Offset;
	};
	struct sGeoKeys         // LASF_Projection 34735
	{
		unsigned short wKeyDirectoryVersion;
		unsigned short wKeyRevision;
		unsigned short wMinorRevision;
		unsigned short wNumberOfKeys;
	};

	//--------------------------------------------

	struct RGB
	{
		unsigned short red;                //      2
		unsigned short green;              //      2
		unsigned short blue;               //      2   
	};

	struct Wave{
		unsigned char wavepacket;          //28      1
		unsigned long long  offsetovaform; //29-36   8      1 
		unsigned long  waveform;           //37-40  4
		float returnpoint;                 //41-44   4   13
		float xt;                          //4
		float yt;                          //4
		float zt;                          //4
	};

	struct Pos
	{
		long  X;                        //0-3         4                                      //0-3
		long  Y;                        //4-7         4                                         //4-7
		long  Z;                        //8-11        4      
	};

	struct PointFormat0
	{
		Pos m_position;				  //0-11
		unsigned short int	m_intensity;      //12-13       2                           
		unsigned char  m_returnNumber : 3;          //14          1                               //14
		unsigned char  m_numOfReturns : 3;
		unsigned char  m_scanDir : 1;
		unsigned char  m_eageFlightLine : 1;
		unsigned char  m_classification;  //15          1         
		char m_scanAngle;                 //16          1  
		unsigned char m_userData;         //17         1  
		unsigned short m_PointSourceID;   //18 -19       2
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
		unsigned short int	m_intensity;       //12-13        2                                   
		unsigned char   m_other[2];        //14-15        2                            
		unsigned char  m_classification;   //16           1    
		unsigned char m_userData;          //17           1  
		short m_scanAngle;                 //18-19        2  
		unsigned short m_PointSourceID;    //20 -21       2
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