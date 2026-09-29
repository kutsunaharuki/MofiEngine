pipeline {
    agent any

    triggers {
        pollSCM('H/5 * * * *')
    }

    environment {
        IMAGE    = 'dx12-buildtools:latest'
        SOLUTION = 'Game\\Game.sln'
        PLATFORM = 'x64'
    }

    options {
        timestamps()
        timeout(time: 60, unit: 'MINUTES')
    }

    stages {
        stage('Build') {
            parallel {
                stage('Debug') {
                    steps {
                        bat """
                        docker run --rm -v ${WORKSPACE}:C:\\src ${IMAGE} cmd /c "msbuild C:\\src\\${SOLUTION} /p:Configuration=Debug /p:Platform=${PLATFORM}"
                        """
                    }
                }
                stage('Release') {
                    steps {
                        bat """
                        docker run --rm -v ${WORKSPACE}:C:\\src ${IMAGE} cmd /c "msbuild C:\\src\\${SOLUTION} /p:Configuration=Release /p:Platform=${PLATFORM}"
                        """
                    }
                }
            }
        }
    }
}