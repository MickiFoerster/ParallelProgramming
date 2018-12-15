pipeline {
  agent any
  stages {
    stage('Init') {
      steps {
        echo "Testing..."
      }
    }
    stage('Build') {
      parallel {
        stage('echoing') {
          steps {
            echo "Building..."
          }
        }
        stage('what hostname do you have?') {
          steps {
            sh 'hostname'
          }
        }
        stage('git version') {
          steps {
            sh '/usr/bin/git --version'
          }
        }
        stage('gcc version') {
          steps {
            sh 'gcc --version'
          }
        }
        stage('g++ version') {
          steps {
            sh 'g++ --version'
          }
        }
      }
    }
    stage('Deploy') {
      steps {
        echo "Code deployed."
      }
    }
  }
}
